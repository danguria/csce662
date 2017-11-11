/*!
 * \file p2p_comm.h
 * \class P2PComm
 *
 * \brief Class for process to process communication using grpc.
 */

#ifndef P2P_COMM_H
#define P2P_COMM_H
#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "ft/p2p_server.h"
#include "ft/p2p_client.h"
#include "utils/log.h"
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

namespace ft {
    class P2PComm {
        private:
            P2PServer *_server;
            vector<P2PClient*> *_local_clients;
            vector<P2PClient*> *_network_clients;
            std::thread _th_server;
            int _local_maxId;
            int _local_minId;

            void removeP2PClient(bool local, int id);

        public:
            static const string TAG;
            bool _ready;
            mutex _lock;

            P2PComm();

            void init(int sid, string server_p2p_addr, string server_chat_addr,
                    map<int, string> local, map<int, string> net);
            vector<P2PClient*>* getP2PClients(bool local) { 
                if (local) return _local_clients;
                else return _network_clients;
            }

            P2PClient* getP2PClient(bool local, int id);
            P2PServer* getP2PServer() { return _server; }

            int getMaxClientId() {
                return _local_maxId;
            }

            int getMinClientId() {
                return _local_minId;
            }

            void resetP2PLocalClient(int id);
            void resetP2PNetMaster(int id, std::string addr);

            std::string getNextChatServerAddr();
    };
}

#endif // P2P_COMM_H
