/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <thread>

#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "chat_manager.h"
#include "chat.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using chatroom::ChatRoomService;
using chatroom::User;
using chatroom::ChatRoom;
using chatroom::JoinRequest;
using chatroom::LeaveRequest;
using chatroom::Reply;
using chatroom::Chat;

fbsd::ChatManager _chatManager;

class ChatRoomServiceForCommand final : public ChatRoomService::Service {
    private:
    public:
    ChatRoomServiceForCommand() {
    }
    Status GetAllChatRooms(ServerContext* context, const User* user, ServerWriter<ChatRoom>* writer) override {
        std::cout << std::endl << ">>>>>> Get rpc GetAllChatRooms by"
            << user->name() << " <<<<<<" << std::endl;

        vector<string> chatRooms = _chatManager.GetAllChatRooms(user->name());

        for (string chatRoom : chatRooms) {
            std::cout << "room: " << chatRoom << std::endl;
            ChatRoom cr;
            cr.set_name(chatRoom);
            writer->Write(cr);
        }

        std::cout << "Status OK" << std::endl;
        return Status::OK;
    }

    Status GetJoinedChatRooms(ServerContext* context, const User* user, ServerWriter<ChatRoom>* writer) override {
 
        std::cout << std::endl << ">>>>>> Get rpc GetJoinedChatRooms by "
            << user->name() << " <<<<<<" << std::endl;

        vector<string> chatRooms = _chatManager.GetJoinedChatRooms(user->name());

        for (string chatRoom : chatRooms) {
            std::cout << "room: " << chatRoom << std::endl;
            ChatRoom cr;
            cr.set_name(chatRoom);
            writer->Write(cr);
        }

        std::cout << "Status OK" << std::endl;
        return Status::OK;
    }

    Status JoinChatRoom(ServerContext* context, const JoinRequest* request, Reply* reply) override {
        
        std::string name = request->sender().name();
        std::string targetchatroom = request->targetchatroom().name();
        
        std::cout << std::endl << ">>>>>> Get rpc JoinChatRoom by "
            << name << " to " << targetchatroom << " <<<<<<" << std::endl;
        int ret;
        ret = _chatManager.JoinChatRoom(
                request->sender().name(),
                request->targetchatroom().name()
                );
        if (ret == fbsd::ChatManager::ERROR_UNKNOWN_USER) {
            reply->set_success(false);
            reply->set_error_message("unknown user room");
        } else if (ret == fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM) {
            reply->set_success(false);
            reply->set_error_message("unknown target room");
        } else if (ret == fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT) {
            reply->set_success(false);
            reply->set_error_message("unknown participant");
        } else if (ret == fbsd::ChatManager::ERROR_ALREADY_JOINED) {
            reply->set_success(false);
            reply->set_error_message("you have already joined");
        } else if (ret == fbsd::ChatManager::SUCCESS) {
            reply->set_success(true);
            reply->set_error_message("success");
        } else {
            reply->set_success(false);
            reply->set_error_message("unknown result");
        }

        std::cout << "Status OK" << std::endl;
        return Status::OK;
    }

    Status LeaveChatRoom(ServerContext* context,
            const LeaveRequest* request, Reply* reply) override {
        
        std::string name = request->sender().name();
        std::string targetchatroom = request->targetchatroom().name();
        
        std::cout << std::endl << ">>>>>> Get rpc  LeaveChatRoom by "
            << name << " to " << targetchatroom << " <<<<<<" << std::endl;

        int ret = _chatManager.LeaveChatRoom(name, targetchatroom);
        if (ret == fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM) {
            reply->set_success(false);
            reply->set_error_message("unkown target chat room");
        } else if (ret == fbsd::ChatManager::ERROR_SELF_LEAVE) {
            reply->set_success(false);
            reply->set_error_message("you have already leaved");
        } else if (ret == fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT) {
            reply->set_success(false);
            reply->set_error_message("unknow participant");
        } else if (ret == fbsd::ChatManager::SUCCESS) {
            reply->set_success(true);
            reply->set_error_message("success");
        } else {
            reply->set_success(false);
            reply->set_error_message("unknown result");
        }
        std::cout << "Result: " << reply->error_message() << std::endl;
        std::cout << "Status OK" << std::endl;
        return Status::OK;
    }

    Status ShowChats(ServerContext* context, const User* user, ServerWriter<Chat>* writer) override {
        std::cout << std::endl << ">>>>>> Get rpc ShowChats for "
            << user->name() << " <<<<<<" << std::endl;
        
        vector<fbsd::Chat*> chats = _chatManager.ShowChats(user->name());
        
        std::cout << "-------  List of chats ------" << std::endl;
        for (fbsd::Chat* chat: chats) {
            std::string sender = chat->GetSender();
            std::string time = chat->GetTimeString();
            std::string message = chat->GetMessage();
            std::cout << "  " << sender << " posted message: " << message << " at " << time << std::endl;

            Chat chat_reply;
            User *u = new User;
            u->set_name(sender);
            chat_reply.set_allocated_name(u);
            chat_reply.set_time(time);
            chat_reply.set_chat(message);

            writer->Write(chat_reply);
        }
        std::cout << "Status OK" << std::endl;
        return Status::OK;
    }
    
    Status PostChat(ServerContext* context, const Chat* request, Chat* chat_result) override {
        std::string sender = request->name().name();
        std::string message = request->chat();
        std::time_t time = std::time(0);
        std::cout << std::endl << ">>>>>> Get rpc PostChat from "
            << sender << " msseage " << message << " at " << ctime(&time) << " <<<<<< " << message << std::endl;
   
        _chatManager.PostChat(sender, message, time);

        User *u = new User;
        u->set_name(sender);
        chat_result->set_allocated_name(u);
        chat_result->set_time(ctime(&time));
        chat_result->set_chat(message);

        std::cout << "Status OK" << std::endl;
        return Status::OK;
    }
};

class ChatRoomServiceForChat final : public ChatRoomService::Service {
    public:
        Status WaitForChat(ServerContext* context, const User* user, Chat* chat_result) override {
            std::cout << std::endl << ">>>>>> Get rpc WaitForChat from "
                << user->name() << " <<<<<< " << std::endl;

            fbsd::Chat* chat = _chatManager.WaitForChat(user->name());

            User *u = new User;
            if (chat != NULL) {
                u->set_name(chat->GetSender());
                chat_result->set_allocated_name(u);
                chat_result->set_time(chat->GetTimeString());
                chat_result->set_chat(chat->GetMessage());

                std::cout << "new message: " << chat->GetMessage()
                    << " from " << chat->GetSender() << " at " << chat->GetTimeString() << std::endl;
            } else {
                u->set_name("there's no such room...");
                chat_result->set_allocated_name(u);
                std::cout << "error: there's no such room..." << std:: endl;;
            }
            std::cout << "Status OK" << std::endl;
            return Status::OK;
        }
};

void RunCommandServer(std::string port) {
    std::string server_address("localhost:" + port);
    ChatRoomServiceForCommand service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "ChatRoom Server listening on " << server_address << std::endl;
    server->Wait();
}

void RunChatServer(std::string port) {
    std::string server_address("localhost:" + port);
    ChatRoomServiceForChat service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "ChatRoom Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {

    if (argc != 3) {
        fprintf(stderr, "usage: fbsd port_command port_chat\n");
        exit(1);
    }

    _chatManager.Init();
    std::thread command_thread(RunCommandServer, argv[1]);
    RunChatServer(argv[2]);

  return 0;
}
