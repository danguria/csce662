/*!
 * \file chatroom_io.h
 * \class ChatRoomIO
 *
 * \brief Internal data structure for the file io
 *
 */
#ifndef CHATROOM_IO_H
#define CHATROOM_IO_H
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "fbsd/chat.h"
#include "fbsd/chat_room.h"

namespace fbsd {
class ChatRoomIO {
    private:
        vector<string> _in;
    public:
        ChatRoomIO() : _in() {
            init();
        }

        void init() {
            _in.clear();
            ifstream file("data.dat");
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    _in.push_back(line);
                }
            }
            file.close();
        }


        /** @brief return a chatroom from the file
          @return ChatRoom*  parsed chatroom from the file
          */
        ChatRoom* ReadNextChatRoom();

    private:
        Participant* ReadParticipant();
        Chat* ReadChat();
        void split(string str, string *left, string* right);
        string ReadOwner();
        bool AssertTag(string tag);
};
}


#endif  // CHATROOM_IO_H
