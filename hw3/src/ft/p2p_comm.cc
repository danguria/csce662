#include "ft/p2p_comm.h"
#include "utils/utils.h"

const string ft::P2PComm::TAG = "P2PComm";

extern int _master;

ft::P2PComm::P2PComm() {
    _ready = false;
    _local_clients = new vector<P2PClient*>;
    _network_clients = new vector<P2PClient*>;
}

void ft::P2PComm::init(int sid, string server_p2p_addr, string server_chat_addr,
        map<int, string> local, map<int, string> net) {

    fbsd::Log::v(TAG, "wait lock in init");
    _lock.lock();
    fbsd::Log::v(TAG, "get lock in init");

    fbsd::Log::d(TAG,
            fbsd::Log::string_format(
                "Creating server with id(%d) p2p(%s) chat(%s)",
                sid, server_p2p_addr.c_str(), server_chat_addr.c_str()));
    _server = new ft::P2PServer(sid, server_p2p_addr, server_chat_addr);
    _th_server = _server->runThread();
    _th_server.detach();

    int maxId = INT_MIN;
    int minId = INT_MAX;

    for (auto info : local) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "Creating client with %d, %s",
                    info.first, info.second.c_str()));
        P2PClient* c = new ft::P2PClient(
                    info.first, info.second,
                    grpc::CreateChannel(info.second,
                        grpc::InsecureChannelCredentials()));
        _local_clients->push_back(c);
        maxId = maxId > info.first? maxId : info.first;
        minId = minId < info.first? minId : info.first;
    }

    _local_maxId = maxId;
    _local_minId = minId;

    std::map<int, std::string> net1, net2;
    fbsd::Utils::getNetMasterInfo(net, net1, net2);

    int net_master = -1;
    std::string net_addr;
    for (auto info : net1) {
        if (net_master < info.first) {
            net_master = info.first;
            net_addr = info.second;
        }
    }

    if (net_master != -1) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "Creating net client with %d, %s",
                    net_master, net_addr.c_str()));
        _network_clients->push_back(new ft::P2PClient(
                    net_master, net_addr, 
                    grpc::CreateChannel(net_addr,
                        grpc::InsecureChannelCredentials())));
    }
    
    net_master = -1;
    for (auto info : net2) {
        if (net_master < info.first) {
            net_master = info.first;
            net_addr = info.second;
        }
    }

    if (net_master != -1) {
        fbsd::Log::d(TAG,
                fbsd::Log::string_format(
                    "Creating net client with %d, %s",
                    net_master, net_addr.c_str()));
        _network_clients->push_back(new ft::P2PClient(
                    net_master, net_addr, 
                    grpc::CreateChannel(net_addr,
                        grpc::InsecureChannelCredentials())));
    }

    _ready = true;
    _lock.unlock();
    fbsd::Log::v(TAG, "release lock in init");
}


ft::P2PClient* ft::P2PComm::getP2PClient(bool local, int id) {
    fbsd::Log::v(TAG, "wait lock in getP2PClient");
    _lock.lock();
    fbsd::Log::v(TAG, "get lock in getP2PClient");
    ft::P2PClient* client = NULL;
    if (local) {
        for (auto c : *_local_clients) {
            if (c->getID() == id) {
                client = c;
                break;
            }
        }
    } else {
        for (auto c : *_network_clients) {
            if (c->getID() == id) {
                client = c;
                break;
            }
        }
    }
    _lock.unlock();
    fbsd::Log::v(TAG, "release lock in getP2PClient");
    return client;
}

void ft::P2PComm::removeP2PClient(bool local, int id) {
    ft::P2PClient *c = NULL;
    if (local) {
        int i;
        for (i = 0; i < _local_clients->size(); i++) {
            c = (*_local_clients)[i];
            if (c->getID() == id) break;
        }
        _local_clients->erase(_local_clients->begin() + i);
    } else {
        int i;
        for (i = 0; i < _network_clients->size(); i++) {
            c = (*_local_clients)[i];
            if (c->getID() == id) break;
        }
        _network_clients->erase(_network_clients->begin() + i);
    }
    if (c != NULL)
        delete c;
}

void ft::P2PComm::resetP2PLocalClient(int id) {
    ft::P2PClient* c = getP2PClient(true, id);
    fbsd::Log::v(TAG, "wait lock in resetP2PLocalClient");
    _lock.lock();
    fbsd::Log::v(TAG, "get lock in resetP2PLocalClient");
    if (c == NULL) {
        std::cerr << "failed to get p2p client : " << id << std::endl;
        _lock.unlock();
        fbsd::Log::v(TAG, "release lock in resetP2PLocalClient");
        return;
    }

    string addr = c->getAddress();
    removeP2PClient(true, id);

    c = new ft::P2PClient(
            id, addr,
            grpc::CreateChannel(addr,
                grpc::InsecureChannelCredentials()));
    _local_clients->push_back(c);
    _lock.unlock();
    fbsd::Log::v(TAG, "release lock in resetP2PLocalClient");
}
            
void ft::P2PComm::resetP2PNetMaster(int id, std::string addr) {

    fbsd::Log::v(TAG, "wait lock in resetP2PNetMaster");
    _lock.lock();
    fbsd::Log::v(TAG, "get lock in resetP2PNetMaster");
    std::string ip, port;
    fbsd::Utils::split(addr, ip, port);

    int idx = -1;
    for (int i = 0; i < _network_clients->size(); i++) {
        P2PClient* c = (*_network_clients)[i];
        std::string iip, pport;
        fbsd::Utils::split(c->getAddress(), iip, pport);
        if (ip == iip) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        P2PClient* c = (*_network_clients)[idx];
        _network_clients->erase(_network_clients->begin() + idx);
        delete c;
    }

    _network_clients->push_back(new ft::P2PClient(id, addr,
                grpc::CreateChannel(addr,
                    grpc::InsecureChannelCredentials())));
    _lock.unlock();
    fbsd::Log::v(TAG, "release lock in resetP2PNetMaster");
}

std::string ft::P2PComm::getNextChatServerAddr() {
    fbsd::Log::v(TAG, "wait lock in getNextServerAddr");
    _lock.lock();
    fbsd::Log::v(TAG, "get lock in getNextServerAddr");
    for (auto c : *_network_clients) {
        std::string addr = c->GetNetMasterChatAddress();
        if (addr != "failed") {
            fbsd::Log::v(TAG, "release lock in getNextServerAddr");
            _lock.unlock();
            return addr;
        }
    }

    _lock.unlock();
    fbsd::Log::v(TAG, "release lock in getNextServerAddr");
    return _server->getChatAddr();
}
