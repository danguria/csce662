
#include "chat_manager.h"


#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "chat.h"
#include "chat_room.h"
using namespace std;

const int fbsd::ChatManager::SUCCESS = 1;
const int fbsd::ChatManager::ERROR_UNKNOWN_USER = -1;
const int fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM = -2;
const int fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT = -3;
const int fbsd::ChatManager::ERROR_ALREADY_JOINED = -4;
const int fbsd::ChatManager::ERROR_SELF_LEAVE = -5;


vector<string> fbsd::ChatManager::GetAllChatRooms(string user) {

    // begin critical section
    lock_manager.lock();
    
    vector<string> allChatRooms;
    for (ChatRoom *chatRoom : _chatRooms)
        allChatRooms.push_back(chatRoom->GetOwner());
   
    WriteChatRoomsToFile();
    // end critical section
    lock_manager.unlock();
    return allChatRooms;
}

vector<string> fbsd::ChatManager::GetJoinedChatRooms(string user) {

    // begin critical section
    lock_manager.lock();
   
    vector<string> ret;
    ChatRoom* chatRoom = GetChatRoom(user);
    if (chatRoom != NULL)
        ret = chatRoom->GetParticipants();

    WriteChatRoomsToFile();
    // end critical section
    lock_manager.unlock();
    return ret;
}

int fbsd::ChatManager::JoinChatRoom(string user, string targetRoom) {
    // begin critical section
    lock_manager.lock();

    int ret = fbsd::ChatManager::SUCCESS;

    ChatRoom* myChatRoom = GetChatRoom(user);
    ChatRoom* targetChatRoom = GetChatRoom(targetRoom);

    if (myChatRoom == NULL) {
        if (user == targetRoom) {

            cout << "Creating new chatroom for " << user << endl;
            myChatRoom = new ChatRoom();
            myChatRoom->SetOwner(user);

            if (myChatRoom->AddParticipant(targetRoom)
                    == fbsd::ChatManager::ERROR_ALREADY_JOINED) {
                ret = fbsd::ChatManager::ERROR_ALREADY_JOINED;
            } else {
                _chatRooms.push_back(myChatRoom);
            }
        } else {
            ret = fbsd::ChatManager::ERROR_UNKNOWN_USER;
        }
    } else if (targetChatRoom == NULL) {
        ret = fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM;
    } else {
        ret =  myChatRoom->AddParticipant(targetRoom);
    }

    WriteChatRoomsToFile();

    // end critical section
    lock_manager.unlock();
    return ret;
}

int fbsd::ChatManager::LeaveChatRoom(string user, string targetRoom) {
    // begin critical section
    lock_manager.lock();

    int ret = fbsd::ChatManager::SUCCESS;
    
    if (user == targetRoom) {
        ret = fbsd::ChatManager::ERROR_SELF_LEAVE;
    } else {
        ChatRoom* chatRoom = NULL;
        chatRoom = GetChatRoom(user);
        if (chatRoom == NULL) {
            ret = fbsd::ChatManager::ERROR_UNKNOWN_USER;
        } else {
            ret = chatRoom->RemoveParticipant(targetRoom);
        }
    }

    WriteChatRoomsToFile();

    // end critical section
    lock_manager.unlock();
    return ret;
}

vector<fbsd::Chat*> fbsd::ChatManager::ShowChats(string user) {
    // begin critical section
    lock_manager.lock();

    vector<fbsd::Chat*> ret;
    ChatRoom* chatRoom = GetChatRoom(user);
    if (chatRoom !=  NULL)
        ret = chatRoom->GetSortedChats();

    WriteChatRoomsToFile();

    // end critical section
    lock_manager.unlock();

    return ret;
}

int fbsd::ChatManager::PostChat(string sender, string message, time_t time) {
    // begin critical section
    lock_manager.lock();

    for (fbsd::ChatRoom* chatRoom : _chatRooms) {
        if (chatRoom->HasJoined(sender) >= 0) {
            fbsd::Chat* chat = new fbsd::Chat(sender, message, time);
            chatRoom->AddChat(chat);
        }
    }

    WriteChatRoomsToFile();

    // end critical section
    lock_manager.unlock();

    return fbsd::ChatManager::SUCCESS;
}
           
fbsd::Chat* fbsd::ChatManager::WaitForChat(string sender) {
    fbsd::Chat* chat = NULL;

    while (true) {
        // begin critical section
        lock_manager.lock();
        
        fbsd::ChatRoom* chatRoom = GetChatRoom(sender);
        if (chatRoom == NULL) break;
        
        chat = chatRoom->GetWaitingPost();
        if (chat != NULL) break;

        // end critical section
        lock_manager.unlock();
        this_thread::sleep_for(chrono::seconds(1));
    }

    WriteChatRoomsToFile();
    // end critical section
    lock_manager.unlock();
    return chat;
}

/*****************************
 * private methods - never be synchronized
 *****************************/
fbsd::ChatRoom* fbsd::ChatManager::GetChatRoom(string roomName) {

    for (fbsd::ChatRoom* room : _chatRooms) {
        if (roomName == room->GetOwner()) {
            return room;
        }
    }
    return NULL;
}


void fbsd::ChatManager::ReadChatRoomsFromFile() {

    fbsd::ChatRoom* chatRoom = _file.ReadNextChatRoom();
    while (chatRoom != NULL) {
        _chatRooms.push_back(chatRoom);
        chatRoom = _file.ReadNextChatRoom();
    }
}

void fbsd::ChatManager::WriteChatRoomsToFile() {
    ofstream *outFile = new ofstream();
    outFile->open("data.dat");

    for (fbsd::ChatRoom* chatRoom : _chatRooms) {
        chatRoom->WriteToFile(outFile);
    }

    outFile->close();
}
