#include "utils/log.h"
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "ft/p2p_comm.h"

extern ft::P2PComm _comm;

mutex fbsd::Log::lock_log;

void fbsd::Log::d(const string& tag, const string& msg) {
    if (!DEBUG) return;

    lock_log.lock();
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%T", timeinfo);

    string log;
    if (_comm._ready) {
        log = string_format("[%d] %s %s:\t%s\n", 
                _comm.getP2PServer()->getID(),
                buffer,
                tag.c_str(),
                msg.c_str());
    } else {
        log = string_format("[ ] %s %s:\t%s\n", 
                buffer,
                tag.c_str(),
                msg.c_str());
    }

    cout << log;
    lock_log.unlock();
}

void fbsd::Log::v(const string& tag, const string& msg) {
    if (!VERBOS) return;

    lock_log.lock();
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%T", timeinfo);

    string log;
    if (_comm._ready) {
        log = string_format("[%d] %s %s:\t%s\n", 
                _comm.getP2PServer()->getID(),
                buffer,
                tag.c_str(),
                msg.c_str());
    } else {
        log = string_format("[ ] %s %s:\t%s\n", 
                buffer,
                tag.c_str(),
                msg.c_str());
    }

    cout << log;
    lock_log.unlock();
}

void fbsd::Log::e(const string& tag, const string& msg) {
    lock_log.lock();
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%T", timeinfo);

    string log;
    if (_comm._ready) {
        log = string_format("[%d] %s %s:\t%s\n", 
                _comm.getP2PServer()->getID(),
                buffer,
                tag.c_str(),
                msg.c_str());
    } else {
        log = string_format("[ ] %s %s:\t%s\n", 
                buffer,
                tag.c_str(),
                msg.c_str());
    }

    cerr << log;
    lock_log.unlock();
}

string fbsd::Log::string_format(const string fmt_str, ...) {
    /* Reserve two times as much as the length of the fmt_str */
    int final_n, n = ((int)fmt_str.size()) * 2;
    unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        /* Wrap the plain char array into the unique_ptr */
        formatted.reset(new char[n]);
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return string(formatted.get());
}
