#include "fbsd/fbsd.h"
#include "fbsd/chatroom_service_imp.h"
#include "utils/log.h"

const std::string fbsd::fbsd::TAG = "FBSD";


fbsd::fbsd::fbsd(std::string addr) : _addr(addr) {}

void fbsd::fbsd::run() {
    std::string server_address(_addr);
    ChatRoomServiceImp service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    Log::d("FBSD", Log::string_format(
                "ChatRoom Server listening on %s", server_address.c_str()));

    server->Wait();
}
