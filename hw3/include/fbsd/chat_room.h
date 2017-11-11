/*!
 * \file chat_room.h
 * \class ChatRoom
 *
 * \brief Internal data structure for the chatroom
 *
 */
#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include <iostream>
#include <string>
#include <vector>
#include "fbsd/participant.h"
#include "fbsd/chat.h"
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
    class ChatRoom {
        private:
            string _owner;
            ServerReaderWriter<Message, Message>* _stream;

            /** these vectors are shared objects, but this should not be
             synchronized because ChatRoom is used by ChatManager that
             is synchronized.
             */
            vector<Participant*> _participants; /**< should not be synchronized */
            vector<Chat*> _chats; /**< should not be synchronized */
            vector<Chat*> _newChats; /**< should not be synchronized */

        public:
            ChatRoom() : _stream(NULL) {}

            string GetOwner() { return _owner; }
            void SetOwner(string owner) { _owner = owner; }
            void SetStream(ServerReaderWriter<Message, Message>* s) {
                _stream = s;
            }
            ServerReaderWriter<Message, Message>* GetStream() {
                return _stream;
            }

            /** @brief check if the user has been joined and return it's location in the list
              @param user    currently connected user name
              @return int    the index of the list, -1 if there isn't such participants
              */
            int HasJoined(string user);

            /** @brief add new participants
              @param  participants    the new participants who want to join
              @return int             return SUCCESS if method succeed,
                                      return ERROR_ALREADY_JOINED if the participants already joined
                                      */
            int AddParticipant(string participant);


            /** @brief add new participants
              @param  participants    the new participants who want to join
              @return int             return SUCCESS if method succeed,
                                      return ERROR_ALREADY_JOINED if the participants already joined
                                      */
            int AddParticipant(Participant* p);


            /** @brief remove participants from the chatroom
              @param  participants    the participants who want to leave
              @return int             return SUCCESS if method succeed,
                                      return ERROR_UNKNOWN_PARTICIPANT if there isn't such participants
                                      */
            int RemoveParticipant(string participant);


            /** @brief getter method for participant
              @return vector<string> the list of participant names
            */
            vector<string> GetParticipants();


            /** @brief add new message to the chartroom
              @param chat new message
              @return int N/A
              */
            int AddChat(Chat* chat);

            /** @brief return recent 20 message
              @return vector<Chat*> list of messages
              */
            vector<Chat*> GetSortedChats();

            /** @brief return new arrived message
              @return Chat* new arried message
              */
            Chat* GetWaitingPost();

            /** @brief write current contents of the chatroom
              @param outFile   file descriptor for writting
              */
            void WriteToFile(ofstream *outFile);

        private:
            void MakeSureChatSize(int n);
    };
}
#endif // CHAT_ROOM_H
