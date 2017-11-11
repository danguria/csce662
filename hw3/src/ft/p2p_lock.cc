#include "ft/p2p_lock.h"
#include "ft/vector_clock.h"
#include <fstream>

extern ft::P2PComm _comm;
extern ft::VectorClock _vc;

const std::string ft::P2PLock::TAG = "P2PLock";

ft::P2PLock::P2PLock() {
		qFileName = "queue.txt";
		STATE = UNKNOWN;
}

void ft::P2PLock::init() {
	STATE = RELEASED;
    allowed = 2;

    std::ifstream qfile(qFileName);
    if (qfile.good()) {
        fbsd::Log::v(TAG, "release all queued request");
        std::string line;
        while (getline (qfile,line)){
            ft::P2PClient* client = _comm.getP2PClient(false, std::stoi(line));
            client->OnReleasedLock(_comm.getP2PServer()->getID());
        }
        qfile.close();
    }
}

void ft::P2PLock::requestLock() {

    STATE = WANTED;
    std::vector<int> time = _vc.requestTime(_comm.getP2PServer()->getID());
            
    fbsd::Log::v(TAG, "Request happens");
    ft::P2PServer* server = _comm.getP2PServer();
    int pid = server->getID();

    vector<ft::P2PClient*>* votingSet = getVotingSet();
    if (votingSet == NULL) {
        std:cerr << "voting set is null" << std::endl;
        return;
    }

    allowed = 0;
    for (auto c : *votingSet) {
        fbsd::Log::v(TAG, fbsd::Log::string_format(TAG,
                    "Call OnRequestedLock to %d", c->getID()));
        if (!c->OnRequestedLock(pid, _vc.toString(time).c_str())) {
            fbsd::Log::v(TAG, fbsd::Log::string_format(TAG,
                        "Waiting reply from %d", c->getID()));
            allowed++;
        } else {
            fbsd::Log::v(TAG, fbsd::Log::string_format(TAG,
                        "Received reply from %d", c->getID()));
        }
    }

    waitForReply();
    STATE = HELD;
}

void ft::P2PLock::releaseLock() {
    STATE = RELEASED;

    for (auto id : QUEUE) {
        ft::P2PClient* client = _comm.getP2PClient(false, id);
        fbsd::Log::v(TAG, fbsd::Log::string_format(TAG,
                    "Call OnRequestedLock to %d", client->getID()));
        client->OnReleasedLock(_comm.getP2PServer()->getID());
    }
    clearQueue();
}

bool ft::P2PLock::onRequestedLock(int pid, string t) {
    int myId = _comm.getP2PServer()->getID();
    if (STATE == HELD
            || (STATE == WANTED && _vc.comparison(_vc.toVector(t), myId))) {
        if (STATE == HELD) {
            fbsd::Log::v(TAG, "Put this request to quque since I am holding the lock");
        } else {
            fbsd::Log::v(TAG, "Put this request to quque since I request the lock earlier.");
        }
        enqueu(pid);
        return false;
    } else {
        fbsd::Log::v(TAG, fbsd::Log::string_format(
                    "I am going to reply to requester %d", pid));
        return true;
    }
}

void ft::P2PLock::onReleasedLock() {
    fbsd::Log::v(TAG, "Received reply from ");
    allowed--;
}

void ft::P2PLock::waitForReply() {
    while(allowed > 0);
}

vector<ft::P2PClient*>* ft::P2PLock::getVotingSet() {
    return _comm.getP2PClients(false);
}

void ft::P2PLock::enqueu(int id) {
    ofstream qfile;
    qfile.open(qFileName, ios::app);
    qfile << id << std::endl;
    qfile.close();
    QUEUE.push_back(id);
}

void ft::P2PLock::clearQueue() {
    QUEUE.clear();
    if (remove(qFileName.c_str()) != 0)
        perror("Error deleting file");
}
