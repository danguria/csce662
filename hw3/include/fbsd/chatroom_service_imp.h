#ifndef CHATROOM_SERVICE_IMP_H_
#define CHATROOM_SERVICE_IMP_H_
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <ctime>
#include <thread>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "ft/p2p_comm.h"
#include "ft/failure_detector.h"
#include "fbsd/chat_manager.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using hw2::ChatRoomService;
using hw2::ListReply;
using hw2::Request;
using hw2::Reply;
using hw2::Message;
using hw2::Process;

namespace fbsd {
class ChatRoomServiceImp final : public ChatRoomService::Service {
    public:
        static const std::string TAG;
        ChatRoomServiceImp();
        Status GetServerAddr(ServerContext *context,
                const Request* request, Reply* reply) override;

        Status List(ServerContext* context, const Request* request,
                ListReply* list_reply) override; 

        Status Join(ServerContext* context,
                const Request* request, Reply* reply)override;

        Status Leave(ServerContext* context,
                const Request* request, Reply* reply) override;

        Status Login(ServerContext *context,
                const Request* request, Reply* reply) override;

        Status ServerAlive(ServerContext *context,
                const Request* request, Reply* reply) override;

        Status Chat(ServerContext* context,
                ServerReaderWriter<Message, Message>* stream) override;

};
};

#endif
