/*!
 * \file log.h
 * \class Log
 *
 * \brief This class is responsible for appending additional information to the debug message so that we distinguish.
 */
#ifndef LOG_H
#define LOG_H
#include <string>
#include <iostream>
#include <mutex>
using namespace std;


#define DEBUG 1
#define VERBOS 0
namespace fbsd {
    class Log {

        public:
            static void d(const string& tag, const string& msg);
            static void v(const string& tag, const string& msg);
            static void e(const string& tag, const string& msg);
            static string string_format(const string fmt_str, ...);
        private:
            static mutex lock_log; /**< lock object for the synchronization of logging system*/
    };
};

#endif // LOG_H
