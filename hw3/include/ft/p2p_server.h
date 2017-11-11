/*!
 * \file p2p_server.h
 * \class P2PServer
 *
 * \brief Class for process to process communication using grpc.
 */

#ifndef P2P_SERVER_H
#define P2P_SERVER_H
#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "utils/log.h"
#include "thread.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Channel;
using hw2::ChatRoomService;
using hw2::ListReply;
using hw2::Request;
using hw2::Reply;
using hw2::Message;
using hw2::Process;
using hw2::Vote;
using hw2::ReqLock;
using hw2::ServerInfo;
using hw2::Command;

namespace ft {

    class P2PServerImpl final
        : public ChatRoomService::Service {
        private:
            int _id;
        public:
            static const string TAG;
            P2PServerImpl(int id);
            Status MasterHeartbeats(ServerContext* context,
                    const Process* request, Process* reply) override;
            Status SlaveWatchdog(ServerContext* context,
                    const Process* request, Process* reply) override;
            Status Election(ServerContext* context,
                    const Vote* vote, Reply* reply) override;
            Status NotifyNewLocalMaster(ServerContext* context,
                    const ServerInfo* sInfo, Reply* reply) override;
            Status NotifyNewLocalSlave(ServerContext* context,
                    const ServerInfo* sInfo, Reply* reply) override;
            Status NotifyNewNetMaster(ServerContext* context,
                    const ServerInfo* sInfo, Reply* reply) override;
            Status OnRequestedLock(ServerContext* context,
                    const ReqLock* request, ReqLock* reply) override;
            Status OnReleasedLock(ServerContext* context,
                    const Process* request, Process* reply) override;
            Status OnDataUpdated(ServerContext* context,
                    const Command* vote, Reply* reply) override;
            Status GetNetMasterChatAddress(ServerContext* context,
                    const ServerInfo* vote, ServerInfo* reply) override;
    };

    class P2PServer : public fbsd::Thread {
        private:
            int _id;
            string _p2p_addr, _chat_addr;
            P2PServerImpl _server;

        public:
            static const string TAG;
            P2PServer(int id, string p2p_addr, string chat_addr);
            int getID() { return _id; }
            void run();
            string getAddr() { return _p2p_addr; }
            string getIP() {
                int idx = _p2p_addr.find(":");
                string ip;
                ip = _p2p_addr.substr(0, idx);
                return ip;
            }

            string getChatAddr() { return _chat_addr; }
    };
    

}
#endif // P2P_SERVER_H
