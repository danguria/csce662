#include "utils/utils.h"

void fbsd::Utils::split(const string& str, string& left, string& right) {
    int idx = str.find(":");
    left = str.substr(0, idx);
    right = str.substr(idx + 1, str.size());
}

void fbsd::Utils::split(const string& str, string& l, string& m, string& r) {
    int idx = str.find(":");
    std::string remain;
    l = str.substr(0, idx);
    remain = str.substr(idx + 1, str.size());

    split(remain, m, r);
}

void fbsd::Utils::getNetMasterInfo(
        const map<int, string>& net,
        map<int, string>& net1,
        map<int, string>& net2) {

    int cnt = 0;
    string base_addr;
    for (auto info : net) {
        base_addr = info.second;
        if (cnt == 0) break;
    }

    string base_ip, base_port;
    split(base_addr, base_ip, base_port);

    for (auto info : net) {
        int id = info.first;
        string addr = info.second;

        string ip, port;
        split(addr, ip, port);

        if (ip == base_ip) {
            net1[id] = addr;
        } else {
            net2[id] = addr;
        }
    }
}
