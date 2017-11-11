#include "ft/election.h"

extern ft::P2PComm _comm;
const string ft::Election::TAG = "Election";

ft::Election::Election() {
    _participated = false;
    _vote = -1;
}

void ft::Election::forward() {
    int v = _vote == -1? _comm.getP2PServer()->getID() : _vote;
    int myId = _comm.getP2PServer()->getID();
    int fwId = myId;
    P2PClient* c = getNextClient(fwId);
    fwId = c->getID();
    if (c == NULL) return;
    fbsd::Log::d(TAG, fbsd::Log::string_format(
                "Trying to foward to %d with vote %d", fwId, myId));
    while (!c->Election(v)) {
        c = getNextClient(fwId);
        fwId = c->getID();
        fbsd::Log::d(TAG, fbsd::Log::string_format(
                    "Trying to foward to %d with vote %d", fwId, myId));
    }
}

int ft::Election::vote(int id) {
    fbsd::Log::v(TAG,
            fbsd::Log::string_format("Start voting with id %d", id));

    _vote = id;

    int myId = _comm.getP2PServer()->getID();
    if (_vote == myId) {
        // I'm new master!!
        fbsd::Log::d(TAG,
                fbsd::Log::string_format("====== I am Master : %d ======", id));
        _vote = -1; // it make election stop
        return _vote;
    }
    if (!_participated) {
        if (id < myId) {
            _vote = myId;
        } else {
            _vote = id;
        }
        fbsd::Log::v(TAG,
                fbsd::Log::string_format("I vote for %d", _vote));
    }
    _participated = true;
    return _vote;
}

ft::P2PClient* ft::Election::getNextClient(int id) {
    int maxId = _comm.getMaxClientId();
    int minId = _comm.getMinClientId();
    int targetId = id + 1;

    vector<ft::P2PClient*>* clients = _comm.getP2PClients(true);
    while (targetId != id) {
        if (targetId > maxId) targetId = minId;
        fbsd::Log::v(TAG, "wait lock in getNextClient");
        _comm._lock.lock();
        fbsd::Log::v(TAG, "get lock in getNextClient");
        for (auto c : *(_comm.getP2PClients(true))) {
            if (targetId == c->getID()) {
                _comm._lock.unlock();
                fbsd::Log::v(TAG, "release lock in getNextClient_");
                return c;
            }
        }
        targetId++;
    }
    fbsd::Log::e(TAG, "failed to getNextClient");
    fbsd::Log::v(TAG, "release lock in getNextClient");
    _comm._lock.unlock();
    return NULL;
}


void ft::Election::init() {
    _vote = -1;
    _participated = false;
}
