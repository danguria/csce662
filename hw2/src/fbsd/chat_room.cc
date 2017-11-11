#include "chat_room.h"
#include "chat_manager.h"

#include <iostream>
#include <string>
#include <vector>
using namespace std;
    
bool fbsd_compare(fbsd::Chat *i, fbsd::Chat* j) {
    return (i->GetTimeRaw() > j->GetTimeRaw()); 
}

int fbsd::ChatRoom::HasJoined(string participant) {
    for (int i = 0; i < _participants.size(); i++) {
        Participant* p = _participants.at(i);
        if (participant.compare(p->GetName()) == 0) {
            return i;
        }
    }
    return -1;
}

int fbsd::ChatRoom::AddParticipant(string participant) {
    int ret = fbsd::ChatManager::SUCCESS;
    if (HasJoined(participant) >= 0) {
        ret = fbsd::ChatManager::ERROR_ALREADY_JOINED;
    } else {
        _participants.push_back(new Participant(participant));
    }
    return ret;
}

int fbsd::ChatRoom::AddParticipant(fbsd::Participant* p) {
    int ret = fbsd::ChatManager::SUCCESS;
    if (HasJoined(p->GetName()) >= 0) {
        ret = fbsd::ChatManager::ERROR_ALREADY_JOINED;
    } else {
        _participants.push_back(p);
    }
    return ret;
}

int fbsd::ChatRoom::RemoveParticipant(string participant) {
    int ret = fbsd::ChatManager::SUCCESS;
    int idx = HasJoined(participant);
    if (idx < 0) {
        ret = fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT;
    } else {
        fbsd::Participant* p = _participants.at(idx);
        _participants.erase(_participants.begin() + idx);
        delete p;
    }
    return ret;
}

vector<string> fbsd::ChatRoom::GetParticipants() {
    vector<string> users;
    for (fbsd::Participant *p : _participants)
        users.push_back(p->GetName());
    return users;
}

int fbsd::ChatRoom::AddChat(Chat* chat) {
    int ret = fbsd::ChatManager::SUCCESS;
    _newChats.push_back(chat);
    return ret;
}

vector<fbsd::Chat*> fbsd::ChatRoom::GetSortedChats() {
    _chats.reserve(_chats.size() + _newChats.size());
    _chats.insert(_chats.end(), _newChats.begin(), _newChats.end());
    _newChats.clear();
    sort(_chats.begin(), _chats.end(), fbsd_compare);
    MakeSureChatSize(20);
    return _chats;
}

fbsd::Chat* fbsd::ChatRoom::GetWaitingPost() {
    fbsd::Chat* chat = NULL;
    if (!_newChats.empty()) {
        chat = _newChats.at(0);
        _newChats.erase(_newChats.begin());
        _chats.push_back(chat);
    }
    return chat;
}

void fbsd::ChatRoom::WriteToFile(ofstream* outFile) {

    *outFile << "Begin ChatRoom\n";
    *outFile << _owner << "\n";

    for (Participant* p : _participants) {
        *outFile << "Begin Participant\n";
        *outFile << "name:" + p->GetName() + "\n";
        *outFile << "time:" + to_string(p->GetJoinedTime()) + "\n";
        *outFile << "End Participant\n";
    }

    for (Chat* chat : _chats) {
        *outFile << "Begin Chat\n";
        *outFile << "sender:" + chat->GetSender() + "\n";
        *outFile << "message:" + chat->GetMessage() + "\n";
        *outFile << "time:" + to_string(chat->GetTimeRaw()) + "\n";
        *outFile << "End Chat\n";
    }

    for (Chat* chat : _newChats) {
        *outFile << "Begin Chat\n";
        *outFile << "sender:" + chat->GetSender() + "\n";
        *outFile << "message:" + chat->GetMessage() + "\n";
        *outFile << "time:" + to_string(chat->GetTimeRaw()) + "\n";
        *outFile << "End Chat\n";
    }

    *outFile << "End ChatRoom\n";
}

/*****************************
 * private methods - never be synchronized
 *****************************/
void fbsd::ChatRoom::MakeSureChatSize(int maxSize) {
    while (_chats.size() >= maxSize) {
        fbsd::Chat* chat = _chats.at(0);
        _chats.erase(_chats.begin());
        delete chat;
    }
}
