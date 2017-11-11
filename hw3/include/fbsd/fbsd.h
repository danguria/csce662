#ifndef FBSD_H_
#define FBSD_H_
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <ctime>
#include <thread>
#include <algorithm>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "fbsd/chat.h"
#include "thread.h"
#include "ft/p2p_comm.h"
#include "ft/failure_detector.h"

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
    class fbsd : public Thread { // class fbsd extends Thread
        public:
            static const std::string TAG;

            fbsd(std::string addr);
            void run();

        private:
            std::string _addr;
    };
};

#endif
