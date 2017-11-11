#ifndef UTILS_H
#define UTILS_H
#include <iostream>
#include <string>
#include <map>
using namespace std;

namespace fbsd {
    class Utils {
        public:
            static void split(const string& str, string& left, string& right);
            static void split(const string& str, string& l, string& m, string& r);
            static void getNetMasterInfo(
                    const map<int, string>& net,
                    map<int, string>& net1,
                    map<int, string>& net2);
    };
};
#endif // UTILS_H
