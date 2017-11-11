#include "ft/p2p_server.h"
#include "ft/p2p_lock.h"
#include "ft/election.h"
#include "ft/failure_detector.h"
#include "ft/vector_clock.h"
#include "utils/utils.h"
#include "fbsd/chat_manager.h"

extern ft::Election _election;
extern ft::FailureDetector _fd;
extern ft::P2PLock _p2plock;
extern ft::VectorClock _vc;
extern ft::P2PComm _comm;
extern fbsd::ChatManager _chatManager;
extern int _master;

const string ft::P2PServerImpl::TAG = "P2PServerImpl";
const string ft::P2PServer::TAG = "P2PServer";

ft::P2PServerImpl::P2PServerImpl(int id) : _id(id) {}

Status ft::P2PServerImpl::MasterHeartbeats(ServerContext* context,
        const Process* request, Process* reply) {
    pid_t pid = request->pid();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc Heartbeats with pid %d <<<<<<", pid);
    fbsd::Log::v(TAG, msg);

    _fd._masterAlive = true;

    reply->set_pid(_id);
    return Status::OK;

}

Status ft::P2PServerImpl::SlaveWatchdog(ServerContext* context,
        const Process* request, Process* reply) {
    pid_t pid = request->pid();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc SlaveWatchdog with pid %d <<<<<<", pid);
    fbsd::Log::v(TAG, msg);

    reply->set_pid(_id);
    return Status::OK;

}

Status ft::P2PServerImpl::Election(ServerContext* context,
        const Vote* vote, Reply* reply) {
    int id = vote->id();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc Election with id %d <<<<<<", id);
    fbsd::Log::d(TAG, msg);

    if (_election.vote(id) != -1) { // forward
        std::thread th_election = _election.runThread();
        th_election.detach();
    } else {  // I am new master!
        if (_master != _comm.getP2PServer()->getID()) {
            std::thread th = _fd.runTakingOverMaster();
            th.detach();
        }
    }
    reply->set_msg("success");
    return Status::OK;

}

Status ft::P2PServerImpl::NotifyNewLocalMaster(ServerContext* context,
        const ServerInfo* sInfo, Reply* reply) {

    // local master changed
    _master = sInfo->new_pid();
    int dead_id = sInfo->dead_pid();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc NotifyNewLocalMaster with master: %d <<<<<<", _master);
    fbsd::Log::d(TAG, msg);

    _fd._masterAlive = true;
    _election.init();

    if (_fd.finished()) {
        _fd.restart();
        std::thread fdThread = _fd.runThread();
        fdThread.detach();
    }

    reply->set_msg("success");
    return Status::OK;
}

Status ft::P2PServerImpl::NotifyNewLocalSlave(ServerContext* context,
        const ServerInfo* sInfo, Reply* reply) {

    // local master changed
    int pid = sInfo->new_pid();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc NotifyNewLocalSlave with id: %d <<<<<<", pid);
    fbsd::Log::d(TAG, msg);

    _comm.resetP2PLocalClient(pid);
    reply->set_msg("success");
    return Status::OK;
}

Status ft::P2PServerImpl::NotifyNewNetMaster(ServerContext* context,
        const ServerInfo* sInfo, Reply* reply) {


    std::string addr = sInfo->addr();
    int dead_id = sInfo->dead_pid();
    int new_id = sInfo->new_pid();

    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc NotifyNewNetMaster with dead_id: %d new_id: %d <<<<<<", dead_id, new_id);
    fbsd::Log::d(TAG, msg);
   
    _comm.resetP2PNetMaster(new_id, addr);
    _vc.notifyNewNetMaster(dead_id, new_id);

    if (_master == _comm.getP2PServer()->getID()) {
        fbsd::Log::v(TAG, "wait lock in NotifyNewNetMaster");
        _comm._lock.lock();
        fbsd::Log::v(TAG, "get lock in NotifyNewNetMaster");
        for (auto c : *(_comm.getP2PClients(true))) {
            std::string cmds, clock;
            int tmp;
            c->NotifyNewNetMaster(dead_id, new_id, addr, cmds, clock, tmp);
        }
        _comm._lock.unlock();
        fbsd::Log::v(TAG, "release lock in NotifyNewNetMaster");

        if (dead_id == -1) 
            dead_id = new_id;
        std::string cmds = _chatManager.getFailedCmds(dead_id);
        std::string clock = _vc.getClock();
        fbsd::Log::d(TAG, fbsd::Log::string_format(
                    "returnning cmds : %s, clock : %s", cmds.c_str(), clock.c_str()));

        reply->set_cmds(cmds);
        reply->set_clock(clock);
        reply->set_id(_master);
    }

    reply->set_msg("success");
    return Status::OK;
}


Status ft::P2PServerImpl::OnRequestedLock(ServerContext* context,
        const ReqLock* request, ReqLock* reply) {
    pid_t pid = request->pid();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc OnRequestedLock with pid %d <<<<<<", pid);
    fbsd::Log::d(TAG, msg);

    string time = request->time();

    if (_p2plock.onRequestedLock(pid, time)) {
        reply->set_pid(-1);
        fbsd::Log::v(TAG, "return -1");
    } else {
        reply->set_pid(-2);
        fbsd::Log::v(TAG, "return -2");
    }

    return Status::OK;

}

Status ft::P2PServerImpl::OnReleasedLock(ServerContext* context,
        const Process* request, Process* reply) {
    pid_t pid = request->pid();
    string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc OnReleasedLock with pid %d <<<<<<", pid);
    fbsd::Log::d(TAG, msg);

    _p2plock.onReleasedLock();

    return Status::OK;

}

Status ft::P2PServerImpl::OnDataUpdated(ServerContext* context,
        const Command* cmd, Reply* reply) {
    std::string command = cmd->command();
    std::string msg = fbsd::Log::string_format(
            ">>>>>> Get rpc OnDataUpdated command %s from %d <<<<<<",
            command.c_str(), cmd->pid());
    fbsd::Log::d(TAG, msg);

    std::string inst_s, params;
    fbsd::Utils::split(command, inst_s, params);

    enum Inst { UNKNOWN, JOIN, LEAVE, POST };

    Inst inst = UNKNOWN;
    if (inst_s == "join") {
        inst = JOIN;
    } else if (inst_s == "leave") {
        inst = LEAVE;
    } else if (inst_s == "post") {
        inst = POST;
    } else { /* ??? */ }

    switch (inst) {
        case JOIN: {
                       std::string user, target;
                       fbsd::Utils::split(params, user, target);
                       int ret;
                       ret = _chatManager.JoinChatRoom(user, target, false);
                       if (ret == fbsd::ChatManager::ERROR_UNKNOWN_USER) {
                           reply->set_msg("unknown user room");
                       } else if (ret == fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM) {
                           reply->set_msg("unknown target room");
                       } else if (ret == fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT) {
                           reply->set_msg("unknown participant");
                       } else if (ret == fbsd::ChatManager::ERROR_ALREADY_JOINED) {
                           reply->set_msg("you have already joined");
                       } else if (ret == fbsd::ChatManager::SUCCESS) {
                           reply->set_msg("success");
                       } else {
                           reply->set_msg("unknown result");
                       }
                       break;
                   }
        case LEAVE: {
                        std::string user, target;
                        fbsd::Utils::split(params, user, target);
                        int ret = _chatManager.LeaveChatRoom(user, target, false);
                        if (ret == fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM) {
                            reply->set_msg("unkown target chat room");
                        } else if (ret == fbsd::ChatManager::ERROR_SELF_LEAVE) {
                            reply->set_msg("you have already leaved");
                        } else if (ret == fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT) {
                            reply->set_msg("unknow participant");
                        } else if (ret == fbsd::ChatManager::SUCCESS) {
                            reply->set_msg("success");
                        } else {
                            reply->set_msg("unknown result");
                        }
                        break;
                    }
        case POST: {
                       std::string user, msg, time_s;
                       fbsd::Utils::split(params, user, msg, time_s);
                       time_t time = std::stoi(time_s);
                       _chatManager.PostChat(user, msg, time, false);
                       Message message;
                       message.set_username(user);
                       message.set_msg(msg);
                       google::protobuf::Timestamp* timestamp =
                           new google::protobuf::Timestamp();
                       timestamp->set_seconds(time);
                       timestamp->set_nanos(0);
                       message.set_allocated_timestamp(timestamp);
                       for (auto follower : _chatManager.GetStreamsJoinedTo(user)) {
                           if (follower != NULL) {
                               follower->Write(message);
                           }
                       }
                       break;
                   }
        default:
                std:cerr << "unknown command: " << inst << std::endl;
    }


    std::string cmds = _chatManager.getFailedCmds(cmd->pid());
    reply->set_cmds(cmds);
    reply->set_msg("success");

    return Status::OK;

}

Status ft::P2PServerImpl::GetNetMasterChatAddress(ServerContext* context,
        const ServerInfo* from, ServerInfo* address) {
    fbsd::Log::d(TAG,
            ">>>>>> Get rpc GetNetMasterChatAddress <<<<<<");

    address->set_addr(_comm.getP2PServer()->getChatAddr());

    return Status::OK;
}
///////////////////////////////////////////////////////////////////////////////////

ft::P2PServer::P2PServer(int id, string p2p_addr, string chat_addr)
    : _id(id), _p2p_addr(p2p_addr), _chat_addr(chat_addr), _server(id) {}

    void ft::P2PServer::run() {
        ServerBuilder builder;
        builder.AddListeningPort(
                _p2p_addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&_server);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "P2P Server listening on %s", _p2p_addr.c_str()));
        server->Wait();
    }
