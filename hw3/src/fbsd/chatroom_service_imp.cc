#include "fbsd/chatroom_service_imp.h"
#include "ft/p2p_comm.h"
#include "utils/log.h"

extern fbsd::ChatManager _chatManager;
extern ft::P2PComm _comm;

const std::string fbsd::ChatRoomServiceImp::TAG = "ChatRoomServiceImp";
fbsd::ChatRoomServiceImp::ChatRoomServiceImp() {
    _chatManager.Init();
}

Status fbsd::ChatRoomServiceImp::GetServerAddr(
        ServerContext *context, const Request* request, Reply* reply) {
    std::string name = request->username();
    fbsd::Log::v(TAG, fbsd::Log::string_format(
            ">>>>>> Get rpc GetServerAddr by %s <<<<<<", name.c_str()));

    std::string addr = _comm.getNextChatServerAddr();
    
    fbsd::Log::v(TAG, fbsd::Log::string_format(
            "Returnning address: %s ", addr.c_str()));

    reply->set_msg(addr);

    return Status::OK;
}

Status fbsd::ChatRoomServiceImp::List(
        ServerContext* context, const Request* request,
        ListReply* list_reply) {

    fbsd::Log::v(TAG, fbsd::Log::string_format(
                ">>>>>> Get rpc GetAllChatRooms by %s <<<<<<",
                (request->username()).c_str()));

    vector<string> achatRooms =
        _chatManager.GetAllChatRooms(request->username());

    for (string achatRoom : achatRooms)
        list_reply -> add_all_rooms(achatRoom);

    fbsd::Log::v(TAG, fbsd::Log::string_format(
                ">>>>>> Get rpc GetJoinedChatRooms by %s <<<<<<",
                (request->username()).c_str()));

    vector<string> jchatRooms =
        _chatManager.GetJoinedChatRooms(request->username());

    for (string jchatRoom : jchatRooms)
        list_reply -> add_joined_rooms(jchatRoom);

    return Status::OK;
}

Status fbsd::ChatRoomServiceImp::Join(
        ServerContext* context, const Request* request, Reply* reply) {

        std::string name = request->username();
        std::string targetchatroom = request->arguments(0);

        fbsd::Log::v(TAG, fbsd::Log::string_format(
                    ">>>>>> Get rpc JoinChatRoom by %s to %s <<<<<<",
                    name.c_str(), targetchatroom.c_str()));
        int ret;
        ret = _chatManager.JoinChatRoom(name, targetchatroom, true);
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

        return Status::OK;
    }

Status fbsd::ChatRoomServiceImp::Leave(ServerContext* context,
        const Request* request, Reply* reply) {

    std::string name = request->username();
    std::string targetchatroom = request->arguments(0);

    fbsd::Log::v(TAG, fbsd::Log::string_format(
                ">>>>>> Get rpc  LeaveChatRoom by %s to %s <<<<<<",
                name.c_str(), targetchatroom.c_str()));

    int ret = _chatManager.LeaveChatRoom(name, targetchatroom, true);
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
    return Status::OK;
}

Status fbsd::ChatRoomServiceImp::Login(
        ServerContext *context, const Request* request, Reply* reply) {

        std::string name = request->username();
        std::string targetchatroom = name;

        fbsd::Log::v(TAG, fbsd::Log::string_format(
                    ">>>>>> Get rpc JoinChatRoom by %s to %s <<<<<<",
                    name.c_str(), targetchatroom.c_str()));
        int ret;
        ret = _chatManager.JoinChatRoom(name, targetchatroom, true);
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
        } // should be examined.

        return Status::OK;
}

Status fbsd::ChatRoomServiceImp::ServerAlive(
        ServerContext *context, const Request* request, Reply* reply) {

        std::string name = request->username();
        std::string targetchatroom = name;

        fbsd::Log::v(TAG, fbsd::Log::string_format(
                    ">>>>>> Get rpc ServerAlive by %s <<<<<<",
                    name.c_str()));
        return Status::OK;
}

Status fbsd::ChatRoomServiceImp::Chat(ServerContext* context,
        ServerReaderWriter<Message, Message>* stream) {
    fbsd::Log::v(TAG, ">>>>>> Get rpc Chat <<<<<<");
    Message message;
    std::string username = message.username();
    std::string msg = message.msg();
    google::protobuf::Timestamp temptime = message.timestamp();
    std::time_t time = temptime.seconds();
    while(stream->Read(&message)) {
        username = message.username();
        msg = message.msg();
        temptime = message.timestamp();
        time = temptime.seconds();

        if(msg == "Set Stream") {
            _chatManager.AddStream(username, stream);
            vector<fbsd::Chat*> newest_twentychats =
                _chatManager.ShowChats(username);

            Message new_msg;
            for (auto chat : newest_twentychats) {
                std::string str = chat->GetTimeString()
                    + " :: " + chat->GetSender()
                    + ":" + chat->GetMessage() + "\n";
                new_msg.set_msg(str);
                stream->Write(new_msg);
            }
        } else if (msg == "Set Stream2") {
            _chatManager.AddStream(username, stream);
        } else {
            _chatManager.PostChat(username, msg, time, true);
            for (auto followers : _chatManager.GetStreamsJoinedTo(username)) {
                if (followers != NULL) {
                    followers->Write(message);
                }
            }
        }
    }
    _chatManager.AddStream(username, NULL);
    return Status::OK;
}
