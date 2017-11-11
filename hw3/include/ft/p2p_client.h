/*!
 * \file p2p_client.h
 * \class P2PClient
 *
 * \brief Class for process to process communication using grpc.
 */
#ifndef P2P_CLIENT_H
#define P2P_CLIENT_H
#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "utils/log.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
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

namespace ft{

    class P2PClient {
        private:
            int _id;
            string _addr;
            std::unique_ptr<ChatRoomService::Stub> _stub;

        public:
            static const string TAG;
            P2PClient(int id, string addr, std::shared_ptr<Channel> channel);
            int getID() { return _id; }
            std::string getAddress() { return _addr; }
            string getIP() {
                int idx = _addr.find(":");
                string ip;
                ip = _addr.substr(0, idx);
                return ip;
            }

            // gRpc
            pid_t MasterHeartbeats(pid_t pid);
            pid_t SlaveWatchdog(pid_t pid);
            bool Election(int id);
            void NotifyNewLocalMaster(int dead_id, int new_id, std::string addr);
            void NotifyNewLocalSlave(int pid);
            void NotifyNewNetMaster(int dead_id, int new_id,
                    std::string addr, std::string& cmds, std::string& clock, int& id);
            bool OnRequestedLock(pid_t pid, std::string time);
            void OnReleasedLock(pid_t pid);
            bool OnDataUpdated(std::string command, std::string& fcmds);
            std::string GetNetMasterChatAddress();
    };
}
#endif // P2P_CLIENT_H
