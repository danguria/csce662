#include "fbsd/chatroom_io.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "fbsd/chat.h"
#include "fbsd/chat_room.h"
using namespace std;

fbsd::ChatRoom* fbsd::ChatRoomIO::ReadNextChatRoom() {

    ChatRoom* chatRoom = NULL;
    Participant* participant = NULL;
    Chat* chat = NULL;
    string owner;

    if (AssertTag("Begin ChatRoom")) {
        chatRoom = new ChatRoom();
        owner = ReadOwner();
        chatRoom->SetOwner(owner);

        while (!AssertTag("End ChatRoom")) {
            if (AssertTag("Begin Participant")) {
                chatRoom->AddParticipant(ReadParticipant());
            } else if (AssertTag("Begin Chat")) {
                chatRoom->AddChat(ReadChat());
            }
        }
    }

    return chatRoom;
}

fbsd::Participant* fbsd::ChatRoomIO::ReadParticipant() {

    string name;
    time_t time;
    while (!AssertTag("End Participant")) {
        string line = _in.at(0);
        _in.erase(_in.begin());

        string left, right;
        split(line, &left, &right);
        if (left == "name") {
            name = right;
        } else if (left == "time") {
            time = stol(right);
        }
    }

    return new fbsd::Participant(name, time);
}

fbsd::Chat* fbsd::ChatRoomIO::ReadChat() {

    string sender;
    string message;
    time_t time;
    while (!AssertTag("End Chat")) {
        string line = _in.at(0);
        _in.erase(_in.begin());

        string left, right;
        split(line, &left, &right);
        if (left == "sender") {
            sender = right;
        } else if (left == "time") {
            time = stol(right);
        } else if (left == "message") {
            message = right;
        }
    }

    return new fbsd::Chat(sender, message, time);
}

void fbsd::ChatRoomIO::split(string str, string *left, string* right) {
    int idx = str.find(":");
    string l, r;
    *left = str.substr(0, idx);
    *right = str.substr(idx + 1, str.size());
}

string fbsd::ChatRoomIO::ReadOwner() {
    string owner = _in.at(0);
    _in.erase(_in.begin());
    return owner;
}

bool fbsd::ChatRoomIO::AssertTag(string tag) {
    if (_in.empty()) return false;

    if (_in.at(0) != tag) {
        return false;
    } else {
        _in.erase(_in.begin());
        return true;
    }
}
