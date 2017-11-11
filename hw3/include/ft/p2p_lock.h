/*!
 * \file p2p_comm.h
 * \class P2PComm
 *
 * \brief Class for process to process communication using grpc.
 */

#ifndef P2PLOCK_H_
#define P2PLOCK_H_
#include <iostream>
#include <memory>
#include <string>
#include <ctime>
#include <vector>
#include <thread>
#include <unistd.h>
#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "ft/p2p_server.h"
#include "ft/p2p_client.h"
#include "ft/p2p_comm.h"
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
	enum State { UNKNOWN, RELEASED, WANTED, HELD };
    class P2PLock: public fbsd::Thread {
        private:
			State STATE;
            std::vector<int> QUEUE;
            int allowed;

            vector<P2PClient*>* getVotingSet();
            void waitForReply();
            void enqueu(int id);
            void clearQueue();

            std::string qFileName;
        public:
            static const std::string TAG;

            P2PLock();
            void init(); 
            void requestLock();
            void releaseLock();
            
            bool onRequestedLock(int pid, std::string time);
            void onReleasedLock();

            void run() {}
    };
}

#endif // #ifndef
