#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstdlib>
#include "ft/failure_detector.h"
#include "ft/election.h"
#include "utils/log.h"
#include "fbsd/fbsd.h"
#include "ft/vector_clock.h"
#include "ft/p2p_lock.h"
#include "fbsd/chat_manager.h"

extern ft::P2PComm _comm;
extern ft::Election _election;
extern ft::P2PLock _p2plock;
extern ft::VectorClock _vc;
extern int _master;
extern fbsd::ChatManager _chatManager;
const string ft::FailureDetector::TAG = "FailureDetector";

ft::FailureDetector::FailureDetector() {}

void ft::FailureDetector::run() {
    fbsd::Log::v(TAG, "The primary worker monitor!");
    while(!finished()) {
        _masterAlive = false;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (!_masterAlive && !_election._participated) {
            fbsd::Log::d(TAG, "======== THE PRIMARY WORKER IS DEAD! ========");
            _election.forward();
            finish();
        } else {
            fbsd::Log::v(TAG, "The primary worker is alive!");
        }
    }
    fbsd::Log::d(TAG, "The priary worker monitoring thread is terminating...");
}


std::thread ft::FailureDetector::runTakingOverMaster() {
    return std::thread([=] { takeOverMaster(); });
}

void ft::FailureDetector::takeOverMaster() {
    fbsd::Log::v(TAG, "TAKING OVER MASTER");

    _p2plock.init();
    _vc.init();
    finish(); // make the master monitoring thread stopped
    std::thread th_masterHeartbeat = runMasterHeartbeat();
    th_masterHeartbeat.detach();

    std::thread th_slaveWatchdog = runSlaveWatchdog();
    th_slaveWatchdog.detach();

    int dead_id = _master;
    _master = _comm.getP2PServer()->getID();
    _vc.notifyNewNetMaster(dead_id, _master);
    // run fbsd
    fbsd::Log::d(TAG, fbsd::Log::string_format(
                "Trying to create fbsd server %s",
                (_comm.getP2PServer()->getChatAddr()).c_str()));
    fbsd::fbsd chatroom(_comm.getP2PServer()->getChatAddr());
    std::thread th_chatroom = chatroom.runThread();
    th_chatroom.detach();

    // notify to local slave servers
    fbsd::Log::v(TAG, "wait lock in takingover1");
    _comm._lock.lock();
    fbsd::Log::v(TAG, "get lock in takingover1");
    for (auto c : *(_comm.getP2PClients(true))) {
        fbsd::Log::d(TAG, fbsd::Log::string_format(
                    "Notify New Local worker to %d", c->getID()));
        c->NotifyNewLocalMaster(dead_id, _master, _comm.getP2PServer()->getAddr());
    }
    _comm._lock.unlock();
    fbsd::Log::v(TAG, "release lock in takingover1");

    // notify to remote master servers
    fbsd::Log::v(TAG, "release lock in takingover2");
    _comm._lock.lock();
    fbsd::Log::v(TAG, "get lock in takingover2");
    for (auto c : *(_comm.getP2PClients(false))) {
        fbsd::Log::d(TAG, fbsd::Log::string_format(
                    "Notify New Net worker to %d with dead_id: %d, new_id : %d", c->getID(), dead_id, _master));

        std::string cmds, clock;
        int id;
        c->NotifyNewNetMaster(dead_id,
                _master,
                _comm.getP2PServer()->getAddr(),
                cmds, clock, id);

        fbsd::Log::v(TAG, "call runFailedCmds");
        _chatManager.runFailedCmds(cmds);
        fbsd::Log::v(TAG, "call setClock");
        _vc.setClock(id, clock);
    }
    _comm._lock.unlock();
    fbsd::Log::v(TAG, "release lock in takingover2");
}


std::thread ft::FailureDetector::runMasterHeartbeat() {
    return std::thread([=] { masterHeartbeat(); });
}

void ft::FailureDetector::masterHeartbeat() {
    fbsd::Log::d(TAG, "Heartbeat starts!");

    while(1) {
        int from = _comm.getP2PServer()->getID();
        int to, id;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        fbsd::Log::v(TAG, fbsd::Log::string_format(
                    "_master : %d", _master));
        fbsd::Log::v(TAG, fbsd::Log::string_format(
                    "server  : %d", _comm.getP2PServer()->getID()));

        fbsd::Log::v(TAG, "wait lock in heartbeat");
        _comm._lock.lock();
        fbsd::Log::v(TAG, "get lock in heartbeat");
        for (auto c : *(_comm.getP2PClients(true))) {
            from = _comm.getP2PServer()->getID();
            to = c->getID();
            fbsd::Log::v(TAG, fbsd::Log::string_format(
                        "Send MasterHeartbeat to %d", to));
            id = c->MasterHeartbeats(from);
        }
        _comm._lock.unlock();
        fbsd::Log::v(TAG, "release lock in heartbeat");
    }
}

std::thread ft::FailureDetector::runSlaveWatchdog() {
    return std::thread([=] { slaveWatchdog(); });
}

void ft::FailureDetector::slaveWatchdog() {
    fbsd::Log::d(TAG, "slaveWhatchdog starts!");

    while(1) {

        int from = _comm.getP2PServer()->getID();
        int to, id;

        std::this_thread::sleep_for(std::chrono::seconds(5));

        fbsd::Log::v(TAG, "wait lock in watchdog");
        _comm._lock.lock();
        fbsd::Log::v(TAG, "get lock in watchdog");
        for (auto c : *(_comm.getP2PClients(true))) {
            
            from = _comm.getP2PServer()->getID();
            to = c->getID();

            fbsd::Log::v(TAG, fbsd::Log::string_format(
                        "Send SlaveWatchdog to %d", to));
            
            id = c->SlaveWatchdog(from);
            if (id == -1) {
                _comm._lock.unlock();
                fbsd::Log::v(TAG, "release lock in watchdog_");

                fbsd::Log::d(TAG,
                        fbsd::Log::string_format(
                            "====== Replica %d has been KILLED, restart it ======", to));
           
                std::string addr = c->getAddress();
                std::string ip, port;
                split(addr, ip, port);

                std::string args = fbsd::Log::string_format("%s %d %d",
                        ip.c_str(),
                        to,
                        _master);

                fbsd::Log::d(TAG,
                        fbsd::Log::string_format("new process with args %s",
                            args.c_str()));
               
                pid_t pid = fork();
                if (pid == 0) { // child
                    execl("./bin/fbsd", "./bin/fbsd",
                            ip.c_str(), /* ip */
                            std::to_string(to).c_str(),/* id */
                            std::to_string(_master).c_str(), /*master id*/
                            0);
                    perror("execl");
                    exit(1);
                }
               
                fbsd::Log::d(TAG,
                        fbsd::Log::string_format("reset p2p client %d", to));
                _comm.resetP2PLocalClient(to);
                
                break;
            }
        }
        _comm._lock.unlock();
        fbsd::Log::v(TAG, "release lock in watchdog");
    }
}
