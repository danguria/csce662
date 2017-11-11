#include "fbsd/chat_manager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "fbsd/chat.h"
#include "fbsd/chat_room.h"
#include "ft/p2p_lock.h"
#include "utils/log.h"
#include "utils/utils.h"
using namespace std;

extern ft::P2PLock _p2plock;
extern ft::P2PComm _comm;

const int fbsd::ChatManager::SUCCESS = 1;
const int fbsd::ChatManager::ERROR_UNKNOWN_USER = -1;
const int fbsd::ChatManager::ERROR_UNKNOWN_TARGETROOM = -2;
const int fbsd::ChatManager::ERROR_UNKNOWN_PARTICIPANT = -3;
const int fbsd::ChatManager::ERROR_ALREADY_JOINED = -4;
const int fbsd::ChatManager::ERROR_SELF_LEAVE = -5;

const std::string fbsd::ChatManager::TAG = "ChatManager";

void fbsd::ChatManager::Init() {
    _file.init();
    ReadChatRoomsFromFile();
}

vector<string> fbsd::ChatManager::GetAllChatRooms(string user) {

    // begin critical section
    lock_manager.lock();
    
    vector<string> allChatRooms;
    for (ChatRoom *chatRoom : _chatRooms)
        allChatRooms.push_back(chatRoom->GetOwner());
   
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

    // end critical section
    lock_manager.unlock();
    return ret;
}

int fbsd::ChatManager::JoinChatRoom(string user, string targetRoom, bool net) {
    // begin critical section
    lock_manager.lock();

    int ret = fbsd::ChatManager::SUCCESS;

    ChatRoom* myChatRoom = GetChatRoom(user);
    ChatRoom* targetChatRoom = GetChatRoom(targetRoom);

    if (myChatRoom == NULL) {
        if (user == targetRoom) {

            fbsd::Log::d(TAG, fbsd::Log::string_format(
                        "Creating new chatroom for %s", user.c_str()));
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

    // end critical section
    lock_manager.unlock();

    WriteChatRoomsToFile("join:" + user + ":" + targetRoom, net);

    return ret;
}

int fbsd::ChatManager::LeaveChatRoom(string user, string targetRoom, bool net) {
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

    // end critical section
    lock_manager.unlock();

    WriteChatRoomsToFile("leave:" + user + ":" + targetRoom, net);

    return ret;
}

vector<fbsd::Chat*> fbsd::ChatManager::ShowChats(string user) {
    // begin critical section
    lock_manager.lock();

    vector<fbsd::Chat*> ret;
    ChatRoom* chatRoom = GetChatRoom(user);
    if (chatRoom !=  NULL)
        ret = chatRoom->GetSortedChats();

    // end critical section
    lock_manager.unlock();

    return ret;
}

int fbsd::ChatManager::PostChat(string sender, string message, time_t time, bool net) {
    // begin critical section
    lock_manager.lock();

    for (fbsd::ChatRoom* chatRoom : _chatRooms) {
        if (chatRoom->HasJoined(sender) >= 0) {
            fbsd::Chat* chat = new fbsd::Chat(sender, message, time);
            chatRoom->AddChat(chat);
        }
    }

    // end critical section
    lock_manager.unlock();

    WriteChatRoomsToFile("post:" + sender + ":" + message + ":"
            + std::to_string(time), net);


    return fbsd::ChatManager::SUCCESS;
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

    // restore the backup file if exists.
    std::ifstream backupFile(backupFileName);
    if (backupFile.good()) {
        fbsd::Log::d(TAG, "Backup file found restore..");
        if (std::rename(backupFileName.c_str(), dataFileName.c_str()) != 0)
            perror( "Error renaming file" );
    }

    fbsd::ChatRoom* chatRoom = _file.ReadNextChatRoom();
    while (chatRoom != NULL) {
        _chatRooms.push_back(chatRoom);
        chatRoom = _file.ReadNextChatRoom();
    }
}

void fbsd::ChatManager::WriteChatRoomsToFile(std::string command, bool net) {
    if (net) {
        lock_manager.lock();
        // Backup data file with new info
        fbsd::Log::d(TAG, "Make Backup file");
        ofstream *backupFile = new ofstream();
        backupFile->open(backupFileName);

        for (fbsd::ChatRoom* chatRoom : _chatRooms)
            chatRoom->WriteToFile(backupFile);
        backupFile->close();

        fbsd::Log::d(TAG, "Trying to acquire lock");
        lock_manager.unlock();

        _p2plock.requestLock();
        fbsd::Log::d(TAG, "Entered critical section");

        // begin critical section
        if (remove(backupFileName.c_str()) != 0)
            perror("Error deleting file");

        ofstream *outFile = new ofstream();
        outFile->open(dataFileName);
        for (fbsd::ChatRoom* chatRoom : _chatRooms) {
            chatRoom->WriteToFile(outFile);
        }
        outFile->close();

        _comm._lock.lock();
        std::string fcmds;
        for (auto c : (*_comm.getP2PClients(false))) {

            fbsd::Log::d(TAG, fbsd::Log::string_format(
                        "Call OnDataUpdated to %d", c->getID()));
            if (!c->OnDataUpdated(command, fcmds)) {
                fbsd::Log::d(TAG, "Failed to all OnDataUpdated, save it and try later");
                ofstream cmdFile;
                cmdFile.open(cmdFileName, ios::app);
                cmdFile << c->getID() << ":" << command << std::endl;
                cmdFile.close();
            }
            runFailedCmds(fcmds);
            fcmds.clear();
        }
        _comm._lock.unlock();

        // end critical section
        fbsd::Log::d(TAG, "Trying to release lock");
        _p2plock.releaseLock();
        fbsd::Log::d(TAG, "Release lock");
    } else {
        lock_manager.lock();
        ofstream *outFile = new ofstream();
        outFile->open(dataFileName);
        for (fbsd::ChatRoom* chatRoom : _chatRooms) {
            chatRoom->WriteToFile(outFile);
        }
        outFile->close();
        lock_manager.unlock();
    }
}

void fbsd::ChatManager::AddStream(string user, ServerReaderWriter<Message, Message>* stream) {
    for (auto chatRoom : _chatRooms)
        if (user == chatRoom->GetOwner())
            chatRoom->SetStream(stream);
}


vector<ServerReaderWriter<Message, Message>*>
fbsd::ChatManager::GetStreamsJoinedTo(string user) {
    // begin critical section
    lock_manager.lock();
    vector<ServerReaderWriter<Message, Message>*> streams;

    for (auto chr : _chatRooms) {
        if (chr->GetOwner() != user) {
            vector<string> ps = chr->GetParticipants();
            for (auto p : ps) {
                if (p == user) {
                    ServerReaderWriter<Message, Message>* s = chr->GetStream();
                    if (s != NULL) streams.push_back(s);
                }
            }
        }
    }

    // end critical section
    lock_manager.unlock();
    return streams;
}

std::string fbsd::ChatManager::getFailedCmds(int id) {
    std::string failedCmds;
    std::vector<std::string> remainedCmds;

    {
        std::ifstream cmdFile(cmdFileName);
        if (cmdFile.good()) {
            std::string line, id_s, cmd;
            while (getline (cmdFile, line)) {
                fbsd::Utils::split(line, id_s, cmd);
                if (std::stoi(id_s) == id) {
                    failedCmds = failedCmds + cmd + ";";
                } else {
                    remainedCmds.push_back(line);
                }
            }
            cmdFile.close();
        }
    }

    if (remainedCmds.empty()) {
        fbsd::Log::d(TAG, "There is no failed cmmands, remove the cmd file");
        if (remove(cmdFileName.c_str()) != 0)
            perror("Error deleting file");
    } else {
        fbsd::Log::d(TAG, "Rewriting remained cmd file");
        std::ofstream cmdFile;
        cmdFile.open(cmdFileName);
        for (auto cmd : remainedCmds)
            cmdFile << cmd << std::endl;
        cmdFile.close();
    }
    return failedCmds;
}


int fbsd::ChatManager::runCmd(std::string cmd) {
    fbsd::Log::d(TAG, fbsd::Log::string_format(
                "Trying to run cmd: %s", cmd.c_str()));
    int ret;
    enum Inst { UNKNOWN, JOIN, LEAVE, POST };
    std::string inst_s, params;
    fbsd::Utils::split(cmd, inst_s, params);
    Inst inst = UNKNOWN;
    if (inst_s == "join") {
        inst = JOIN;
    } else if (inst_s == "leave") {
        inst = LEAVE;
    } else if (inst_s == "post") {
        inst = POST;
    } else { /* ??? */ }

    switch (inst) {
        case JOIN: {
                       std::string user, target;
                       fbsd::Utils::split(params, user, target);
                       ret = JoinChatRoom(user, target, false);
                       break;
                   }
        case LEAVE: {
                        std::string user, target;
                        fbsd::Utils::split(params, user, target);
                        ret = LeaveChatRoom(user, target, false);
                        break;
                    }
        case POST: {
                       std::string user, msg, time_s;
                       fbsd::Utils::split(params, user, msg, time_s);
                       time_t time = std::stoi(time_s);
                       PostChat(user, msg, time, false);
                       Message message;
                       message.set_username(user);
                       message.set_msg(msg);
                       google::protobuf::Timestamp* timestamp =
                           new google::protobuf::Timestamp();
                       timestamp->set_seconds(time);
                       timestamp->set_nanos(0);
                       message.set_allocated_timestamp(timestamp);
                       for (auto follower : GetStreamsJoinedTo(user)) {
                           if (follower != NULL) {
                               follower->Write(message);
                           }
                       }
                       break;
                   }
        default:
                std:cerr << "unknown command: " << inst << std::endl;
    }

    return ret;
}

void fbsd::ChatManager::runFailedCmds(std::string cmds) {
    for (auto cmd : parseCmds(cmds)) {
        runCmd(cmd);
    }
}

std::vector<std::string> fbsd::ChatManager::parseCmds(std::string cmds) {

    std::vector<std::string> commands;
    std::string cmd;
    while (!cmds.empty()) {
        int idx = cmds.find(";");
        cmd = cmds.substr(0, idx);
        cmds = cmds.substr(idx + 1, cmds.size());
        commands.push_back(cmd);
    }
    return commands;
}
