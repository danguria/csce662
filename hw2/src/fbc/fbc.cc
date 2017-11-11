#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <thread>
#include <vector>

#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using chatroom::User;
using chatroom::ChatRoom;
using chatroom::JoinRequest;
using chatroom::LeaveRequest;
using chatroom::Reply;
using chatroom::Chat;
using chatroom::ChatRoomService;

char* gArvg;
int cmdModeInterface(std::string user);

void displayChatMode(std::string user);
void displayRoomList(std::vector<std::string> entireList, std::vector<std::string> joinedList);
void displayChat(Chat* chat);
void displayPrevChats(std::vector<Chat> chats);
std::string getRoomName();
void pressEnterToContinue();
std::string trim(const std::string& str);
std::string trim_right(const std::string& str);
std::string trim_left(const std::string& str);


class ImpClientSub {
    public:

        ImpClientSub(std::shared_ptr<Channel> channel)
            : stub_(ChatRoomService::NewStub(channel)){}

        // inChatroom waiting other users messages.
        void WaitChat (std::string userName){
            User user;
            Chat rev_from_server;
            user.set_name(userName);

            do{
                ClientContext s_context;
                Status s_status = stub_->WaitForChat(&s_context, user, &rev_from_server);
                if (s_status.ok()) {
                    std::cout << std::endl;
                    displayChat(&rev_from_server);
                }else { 
                    std::cout << s_status.error_code() << ": " << s_status.error_message() << std::endl;
                } 
            }while(true);
        }

    private:
        std::unique_ptr<ChatRoomService::Stub> stub_;
};

class ImpClient {
    public:

        ImpClient(std::shared_ptr<Channel> channel)
            : stub_(ChatRoomService::NewStub(channel)){}

        //-------showList function-------
        void showList(std::string userName){
            User user;
            ChatRoom roomlist;

            user.set_name(userName);

            //---------All created chatrooms list--------
            ClientContext context;

            std::unique_ptr<ClientReader<ChatRoom> > reader(stub_->GetAllChatRooms(&context, user));

            std::vector<std::string> entireList;
            while (reader->Read(&roomlist))
                entireList.push_back(roomlist.name());

            Status status = reader->Finish();

            if(!status.ok()){
                std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
            }

            //---------Joined chatrooms list--------
            ClientContext s_context;

            std::unique_ptr<ClientReader<ChatRoom> > s_reader(stub_->GetJoinedChatRooms(&s_context, user));

            std::vector<std::string> joinedList;
            while (s_reader->Read(&roomlist))
                joinedList.push_back(roomlist.name());

            Status s_status = s_reader->Finish();

            if(!s_status.ok()){
                std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
            }

            displayRoomList(entireList, joinedList);
        }

        //-------joinRoom function-------
        void joinRoom(std::string userName, int flag){ // if flag is 0 it is the initial join call.
            std::string roomName;
            JoinRequest request;
            Reply reply;
            request.mutable_sender()->set_name(userName);
            ClientContext context;

            if (flag == 0) { // self join
                request.mutable_targetchatroom()->set_name(userName);
                Status status = stub_->JoinChatRoom(&context, request, &reply);
                if (!status.ok()) {
                    std::cout << status.error_code() << ": " << status.error_message() << std::endl; 
                }
            } else {
                roomName = getRoomName();
                request.mutable_targetchatroom()->set_name(roomName);
                Status status = stub_->JoinChatRoom(&context, request, &reply);

                if (status.ok()) {
                    if (reply.success()) {
                        std::cout << "you are successfully joined to : " << roomName << std::endl;
                    } else {
                        std::cout << reply.error_message() << std::endl;
                    }
                } else {
                    std::cout << status.error_code() << ": " << status.error_message() << std::endl; 
                }
            }
        }

        //-------leaveRoom function-------
        void leaveRoom(std::string userName){
            bool out;
            std::string roomName;
            LeaveRequest request;
            Reply reply;
            request.mutable_sender()->set_name(userName);
            ClientContext context;

            roomName = getRoomName();
            request.mutable_targetchatroom()->set_name(roomName);
            Status status = stub_->LeaveChatRoom(&context, request, &reply);
            if (status.ok()) {
                if (reply.success()) {
                    std::cout << "you successfully left : " << roomName << std::endl;
                } else {
                    std::cout << reply.error_message() << std::endl;
                }
            } else {
                std::cout << status.error_code() << ": " << status.error_message() << std::endl; 
            }
        }

        //-------chatInRomm function-------
        void chatInRoom(std::string userName){
            User user;
            Chat pre_chats;
            Chat user_chat;
            Chat rev_from_server;
            pid_t pid;

            user.set_name(userName);

            //--------presenting last 20 chats
            pre_chats.mutable_name()->set_name(userName);
            ClientContext context;

            std::unique_ptr<ClientReader<Chat> > reader(stub_->ShowChats(&context, user));
            std::vector<Chat> chats;
            while (reader->Read(&pre_chats)) {
                chats.push_back(pre_chats);
            }
            displayPrevChats(chats);

            Status status = reader->Finish();

            if(!status.ok()){
                std::cout << status.error_code() <<": " << status.error_message() << std::endl;
            }

            //--------posting current user's comment
            std::string message;
            do{
                ClientContext s_context;
                user_chat.mutable_name()->set_name(userName);
                std::cout << "Enter your message : ";
                std::getline(std::cin, message);
                user_chat.set_chat(message);
                Status s_status = stub_->PostChat(&s_context, user_chat, &rev_from_server);
                if (!status.ok()) {
                    std::cout << status.error_code() << ": " << status.error_message() << std::endl;
                }   
            }while(true);
        }

    private:
        std::unique_ptr<ChatRoomService::Stub> stub_;
};

void thread_func(ImpClientSub* sub) {
    while(true) {
        sub->WaitChat(gArvg);
    }
}

int main(int argc, char* argv[]) {

    int n;
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0] << " HOSTNAME "<< "PORT_COMM" << "PORT_CHAT" << "USER_NAME.\n";
        return -1;
    }

    std::string hostName(argv[1]);
    std::string portNum(argv[2]);
    std::string subPortNum(argv[3]);
    std::string userName = argv[4];

    gArvg = argv[4];

    std::string targetServer = hostName + ":" + portNum;
    std::string sub_targetServer = hostName + ":" + subPortNum;

    ImpClient* client = new ImpClient(grpc::CreateChannel(
                targetServer, grpc::InsecureChannelCredentials()));
    ImpClientSub* sub_client = new ImpClientSub(grpc::CreateChannel(
                sub_targetServer, grpc::InsecureChannelCredentials()));

    client->joinRoom(argv[4],0);

    // socket function
    do {
        n = cmdModeInterface(argv[4]);

        if(n == 1){
            client->showList(userName);
        }
        else if(n == 2){
            client->joinRoom(argv[4],1);
        }      
        else if(n == 3){
            client->leaveRoom(argv[4]);
        }
        else if(n == 4){
            displayChatMode(argv[4]);
            std::thread new_thread(thread_func, sub_client);
            client->chatInRoom(argv[4]);
        }
        pressEnterToContinue();
    }while(true); 


    return 0;
} 

int cmdModeInterface (std::string user) {
    std::cout << "******************************************" << std::endl;
    std::cout << "*       Improved Chat Room Client        *" << std::endl;
    std::cout << "*          (COMMAND MODE)                *" << std::endl;
    std::cout << "******************************************" << std::endl << std::endl;
    std::cout << "Hey " << user << ", Type the number you want to command" << std::endl;
    std::cout << "LIST(1) JOIN(2) LEAVE(3) CHAT(4)" << std::endl << std::endl;

    std::string input;
    int n;
    while (true) {
        std::cout << "Prompt: ";
        try {
            std::cin.clear();
            input.clear();

            std::getline(std::cin, input);
            n = std::stoi(input);
            if ( n < 1 || n > 4) throw "enter between 1 and 4";
            break;
        } catch (const char* msg) {
            std::cout << msg << std::endl;
        } catch( ... ) {
            std::cout << "invalid input, try again..." << std::endl;
        }
    }

    return n;
}

void displayChatMode(std::string user) {
    system("clear");
    std::cout << "******************************************" << std::endl;
    std::cout << "*       Improved Chat Room Client        *" << std::endl;
    std::cout << "*             (CHAT MODE)                *" << std::endl;
    std::cout << "******************************************" << std::endl << std::endl;
    std::cout << "Hey " << user << ", you enetered Chat mode" << std::endl;
}

void displayRoomList(std::vector<std::string> entireList, std::vector<std::string> joinedList) {
    std::cout << "All created chatrooms : {";
    if (entireList.empty())
        std::cout << "none";
    else
        for (std::string room : entireList) std::cout << room << ", ";
    std::cout << "}" << std::endl;

    std::cout << "Joined chatrooms : {";
    if (joinedList.empty())
        std::cout << "none";
    else
        for (std::string room : joinedList) std::cout << room << ", ";
    std::cout << "}" << std::endl;
}

void displayChat(Chat* chat) {

    std::string time = chat->time();
    time = trim(time);
    std::cout << chat->mutable_name()->name()
        << "(" << time << ")"
        << " >> " << chat->chat()
        << std::endl;
}

void displayPrevChats(std::vector<Chat> chats) {
    if (chats.empty()) {
        std::cout << "There aren't recent chats" << std::endl;
    } else {
        std::cout << "Recent 20 chats are ..." << std::endl;
        for (Chat chat : chats) {
            displayChat(&chat);
        }
        std::cout << std::endl;
    }
}

std::string getRoomName() {
    std::string roomName;
    std::cout << "Enter the roomname(username) you want to join: ";
    std::getline(std::cin, roomName);
    return roomName;
}

void pressEnterToContinue() {
    std::cout << "press enter to continue...";
    std::cin.get();
    std::cout << std::endl << std::endl;
}

std::string trim_left(const std::string& str) {
      const std::string pattern = " \f\n\r\t\v";
        return str.substr(str.find_first_not_of(pattern));
}

std::string trim_right(const std::string& str) {
      const std::string pattern = " \f\n\r\t\v";
        return str.substr(0,str.find_last_not_of(pattern) + 1);
}

std::string trim(const std::string& str) {
      return trim_left(trim_right(str));
}
