/*!
 * \file chat_manager.h
 * \class ChatManager
 *
 * \brief Provide methods for managing chatrooms
 *
 * This class is mean for wrapping core classes such as ChatRoom and Chat, etc. This class should be instantiated only once (singleton)
 *
 */
#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "fbsd/chat.h"
#include "fbsd/chat_room.h"
#include "fbsd/chatroom_io.h"
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>
#include "chatroom.grpc.pb.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using hw2::ChatRoomService;
using hw2::ListReply;
using hw2::Request;
using hw2::Reply;
using hw2::Message;
using namespace std;

namespace fbsd {
    class ChatManager {
        public:
            static const int SUCCESS;
            static const int ERROR_UNKNOWN_USER;
            static const int ERROR_UNKNOWN_TARGETROOM;
            static const int ERROR_UNKNOWN_PARTICIPANT;
            static const int ERROR_ALREADY_JOINED;
            static const int ERROR_SELF_LEAVE;
            static const std::string TAG;

            ChatManager() {
                dataFileName = "data.dat";
                backupFileName = "data.dat.bak";
                moreBackupFileName = "data.dat.bak.bak";
                cmdFileName = "cmd.txt";
            }
            /** @brief initialize the chat manager

              In this method, the chat manager read chatroom information from the file 'data.dat'
              */
            void Init();

            /** @brief return all chatroom names

              @param  user            currently connected user name
              @return vector<string>  list of the all chatroom names

*/
            vector<string> GetAllChatRooms(string user);


            /** @brief return chatroom names that current user has been joined.

              @param  user           currently connected user name
              @return vector<string> list of the all chatroom names that user joined.
              */
            vector<string> GetJoinedChatRooms(string user);


            /** @brief try to join to the target chatroom

              @param  user           currently connected user name
              @param  targetRoom     chatroom name that user want to join
              @return int            return SUCCESS if join command succeed,
              ERROR_UNKNOWN_USER if there's no such user,
              ERROR_UNKNOWN_TARGETROOM if there's no such target room,
              ERROR_ALREADY_JOINED if the user already joined
              */
            int JoinChatRoom(string user, string targetRoom, bool net);


            /** @brief make user leave from the chatroom

              @param  user           currently connected user name
              @param  targetRoom     chatroom name that user want to leave
              @return int            return SUCCESS if join command succeed,
              ERROR_UNKNOWN_USER if there's no such user,
              ERROR_UNKNOWN_TARGETROOM if there's no such target room,
              ERROR_SELF_LEAVE  if the user tries to join his/her own chatroom.
              */
            int LeaveChatRoom(string user, string targetRoom, bool net);


            /** @brief return recent twenty chats from the chatrooms that the user whating

              @param user            currently connected user name
              @return vector<Chat*>  list of chats
              */
            vector<Chat*> ShowChats(string user);

            /** @brief post a message to users that have been joined this chatroom

              @param sender           currently connected user name
              @param message          message that the user want to send
              @param time             time that message posted
              @return                 N/A
              */
            int PostChat(string sender, string message, time_t time, bool net);

            void AddStream(string user, ServerReaderWriter<Message, Message>* stream);
            vector<ServerReaderWriter<Message, Message>*> GetStreamsJoinedTo(string user);
            void SendCmds();
            std::string getFailedCmds(int pid);
            int runCmd(std::string cmd);
            void runFailedCmds(std::string cmds);
            std::vector<std::string> parseCmds(std::string cmds);

        private:
            ChatRoom* GetChatRoom(string roomName);
            void ReadChatRoomsFromFile();
            void WriteChatRoomsToFile(std::string command, bool net);

            std::string dataFileName;
            std::string backupFileName;
            std::string moreBackupFileName;
            std::string cmdFileName;

            vector<ChatRoom*> _chatRooms;  /**< list of chat rooms */
            ChatRoomIO _file;   /**< file IO class to read/write the contents of chatroom */
            mutex lock_manager; /**< lock object for the synchronization of manager */
    };

}

#endif  // CHATMANAGER_H
