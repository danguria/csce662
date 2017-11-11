#ifndef MESSENGER_CLIENT_H
#define MESSENGER_CLIENT_H

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using hw2::Message;
using hw2::ListReply;
using hw2::Request;
using hw2::Reply;
using hw2::ChatRoomService;
//Helper function used to create a Message object given a username and message

namespace fbc {
class MessengerClient {
    public:
        MessengerClient(std::shared_ptr<Channel> channel)
            : stub_(ChatRoomService::NewStub(channel)) {}

        //Calls the List stub function and prints out room names
        bool List(const std::string& username);

        //Calls the Join stub function and makes user1 follow user2
        bool Join(const std::string& username1, const std::string& username2);

        //Calls the Leave stub function and makes user1 no longer follow user2
        bool Leave(const std::string& username1, const std::string& username2);

        //Called when a client is run
        std::string Login(const std::string& username);

        bool ServerAlive(const std::string& username);

        //Calls the Chat stub function which uses a bidirectional RPC to communicate
        void Chat (const std::string& username);

        std::string GetServerAddr(std::string username);

        Message MakeMessage(const std::string& username, const std::string& msg);
    private:
        std::unique_ptr<ChatRoomService::Stub> stub_;

};
}

#endif
