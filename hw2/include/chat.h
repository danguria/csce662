/*!
 * \file chat.h
 * \class Chat
 *
 * \brief Internal data structure for the chat message
 */
#ifndef CHAT_H
#define CHAT_H


#include <string>
#include <ctime>
using namespace std;

namespace fbsd {
    class Chat {
        private:
            string _sender;
            string _message;
            time_t  _time;
        public:
            Chat(string sender, string message, time_t time)
                : _sender(sender), _message(message), _time(time) {}

            string GetSender() { return _sender; }
            string GetMessage() { return _message; }
            time_t GetTimeRaw() { return _time; }
            string GetTimeString() { return string(ctime(&_time)); }

    };
};
#endif // CHAT_H
