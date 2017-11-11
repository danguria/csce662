#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <memory>
#include "fbsd/fbsd.h"
#include "fbsd/chat_manager.h"
#include "ft/p2p_comm.h"
#include "ft/election.h"
#include "ft/failure_detector.h"
#include "ft/p2p_lock.h"
#include "ft/vector_clock.h"
using namespace std;

ft::P2PComm _comm;
ft::FailureDetector _fd;
ft::Election _election;
ft::P2PLock _p2plock;
ft::VectorClock _vc;
fbsd::ChatManager _chatManager;

int _master = -1;

static void split(const string& str, string& left, string& right) {
    int idx = str.find(":");
    string l, r;
    left = str.substr(0, idx);
    right = str.substr(idx + 1, str.size());
}

int main(int argc, char** argv) {

    if (argc != 4) {
        fprintf(stderr, "usage: fbsd ip id msterid\n");
        exit(1);
    }

    _master = std::stoi(argv[3]);
    int myID = std::stoi(argv[2]);
    string machine_ip(argv[1]);

    // parse server process info
    ifstream pdata("server-cfg.txt");
    string server_p2p_addr, server_chat_addr;
    map<int, string> local_cInfos;
    map<int, string> net_cInfos;

    string line, id, addr, ip, ports, p2p_port, chat_port;
    if (pdata.is_open()) {
        while (std::getline(pdata, line)) {
            split(line, id, addr);
            split(addr, ip, ports);
            split(ports, p2p_port, chat_port);

            if (myID == std::stoi(id)) {
                server_p2p_addr = ip + ":" + p2p_port;
                server_chat_addr = ip + ":" + chat_port;
            } else {
                if (line.find(machine_ip) != string::npos) { // local address
                    local_cInfos[std::stoi(id)] = ip + ":" + p2p_port;
                } else { // over the network
                    net_cInfos[std::stoi(id)] = ip + ":" + p2p_port;
                }
            }
        }
    }

    _comm.init(myID, server_p2p_addr, server_chat_addr, local_cInfos, net_cInfos);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    if (myID == std::stoi(argv[3])) {
        // take over the master's role
        fbsd::Log::d("FBSD", fbsd::Log::string_format(
                    "ID: %d run as a primary worker", myID));
        std::thread th = _fd.runTakingOverMaster();
        th.detach();
    } else {
        // run failure detector
        fbsd::Log::d("FBSD", fbsd::Log::string_format(
                    "ID: %d run as a slave", myID));
        std::thread fdThread = _fd.runThread();
        fdThread.detach();

        fbsd::Log::v("FBSD", "wait lock in notifylocalslave");
        _comm._lock.lock();
        fbsd::Log::v("FBSD", "get lock in notifylocalslave");
        for (auto c : *(_comm.getP2PClients(true))) {
            fbsd::Log::d("FBSD", fbsd::Log::string_format(
                        "Notify New Local Slave to %d", c->getID()));
            c->NotifyNewLocalSlave(myID);
        }
        _comm._lock.unlock();
        fbsd::Log::v("FBSD", "release lock in notifylocalslave");

    }
    while(1);

    return 0;
}
