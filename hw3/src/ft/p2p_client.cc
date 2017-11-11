#include "ft/p2p_client.h"
#include "ft/p2p_comm.h"
#include "ft/failure_detector.h"

extern ft::P2PComm _comm;

const string ft::P2PClient::TAG = "P2PClient";

ft::P2PClient::P2PClient(int id, string addr, std::shared_ptr<Channel> channel)
    :  _id(id), _addr(addr), _stub(ChatRoomService::NewStub(channel)) {
    }

pid_t ft::P2PClient::MasterHeartbeats(pid_t pid) {

    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    Process request, reply;
    request.set_pid(pid);
    Status status = _stub->MasterHeartbeats(&ctx, request, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
        return -1;
    }
    return reply.pid();
}

pid_t ft::P2PClient::SlaveWatchdog(pid_t pid) {

    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    Process request, reply;
    request.set_pid(pid);
    Status status = _stub->SlaveWatchdog(&ctx, request, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
        return -1;
    }
    return reply.pid();
}

bool ft::P2PClient::Election(int id) {
    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    Vote vote;
    Reply reply;
    vote.set_id(id);
    fbsd::Log::v(TAG,
            fbsd::Log::string_format(
                "election forwarding with vote: %d", id));
    Status status = _stub->Election(&ctx, vote, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
        return false;
    }
    return true;
}

void ft::P2PClient::NotifyNewLocalMaster(int dead_id, int new_id, std::string addr) {
    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    ServerInfo sInfo;
    Reply reply;

    sInfo.set_dead_pid(dead_id);
    sInfo.set_new_pid(new_id);
    sInfo.set_addr(addr);
    fbsd::Log::v(TAG,
            fbsd::Log::string_format(
                "Notifying new local master  with id: %d", new_id));
    Status status = _stub->NotifyNewLocalMaster(&ctx, sInfo, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
    }
}

void ft::P2PClient::NotifyNewLocalSlave(int pid) {
    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    ServerInfo sInfo;
    Reply reply;

    sInfo.set_new_pid(pid);
    fbsd::Log::v(TAG,
            fbsd::Log::string_format(
                "Notifying new local slave with id: %d", pid));
    Status status = _stub->NotifyNewLocalSlave(&ctx, sInfo, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
    }
}

void ft::P2PClient::NotifyNewNetMaster(int dead_id, int new_id,
        std::string addr, std::string& cmds, std::string& clock, int& id) {
    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    ServerInfo sInfo;
    Reply reply;

    sInfo.set_dead_pid(dead_id);
    sInfo.set_new_pid(new_id);
    sInfo.set_addr(addr);
    fbsd::Log::v(TAG,
            fbsd::Log::string_format(
                "Notifying new net master  with id: %d", new_id));
    Status status = _stub->NotifyNewNetMaster(&ctx, sInfo, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
    } else {
        cmds = reply.cmds();
        clock = reply.clock();
        id = reply.id();
    }
}

bool ft::P2PClient::OnRequestedLock(pid_t pid, std::string time) {

    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline); // TODO: Is 30 secs reasonable?

    ReqLock request, reply;
    request.set_pid(pid);
    request.set_time(time);
    Status status = _stub->OnRequestedLock(&ctx, request, &reply);

	int allowed = reply.pid();	// -1: allowed, other: not allowed

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
        return true;  // we don't know whether there's net master or just network problem
    }

    return allowed == -1;
}

void ft::P2PClient::OnReleasedLock(pid_t pid) {

    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    Process request, reply;
    request.set_pid(pid);
    Status status = _stub->OnReleasedLock(&ctx, request, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
    }
}

bool ft::P2PClient::OnDataUpdated(std::string command, std::string& fcmds) {

    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);

    Command cmd;
    Reply reply;
    cmd.set_command(command);
    cmd.set_pid(_comm.getP2PServer()->getID());
    Status status = _stub->OnDataUpdated(&ctx, cmd, &reply);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
        return false;
    }

    fcmds = reply.cmds();
    //clock = reply.clock();  // TODO: need it?

    return true;
}

std::string ft::P2PClient::GetNetMasterChatAddress() {
    ClientContext ctx;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    ctx.set_deadline(deadline);
   
    ServerInfo from, address;
    Status status = _stub->GetNetMasterChatAddress(&ctx, from, &address);

    if (!status.ok()) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "%d: %s", status.error_code(),
                    status.error_message().c_str()));
        return "failed";
    }

    return address.addr();
}
