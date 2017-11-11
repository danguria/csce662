#include "fbc/messenger_client.h"
#include <atomic>

void ReConnectToServer(std::string username);

std::atomic<bool> first_chat(true);
int connected(2);
std::shared_ptr<ClientReaderWriter<Message, Message>> _stream;
Message MakeMessage2(const std::string& username, const std::string& msg) {
    Message m;
    m.set_username(username);
    m.set_msg(msg);
    google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
    timestamp->set_seconds(time(NULL));
    timestamp->set_nanos(0);
    m.set_allocated_timestamp(timestamp);

    return m;
}

bool fbc::MessengerClient::List(const std::string& username){
    //Data being sent to the server
    Request request;
    request.set_username(username);

    //Container for the data from the server
    ListReply list_reply;

    //Context for the client
    ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    context.set_deadline(deadline);

    Status status = stub_->List(&context, request, &list_reply);

    if (status.ok()) {
        //Loop through list_reply.all_rooms and list_reply.joined_rooms
        //Print out the name of each room 
        std::cout << "All Rooms: \n";
        for(std::string s : list_reply.all_rooms()){
            std::cout << s << std::endl;
        }
        std::cout << "Following: \n";
        for(std::string s : list_reply.joined_rooms()){
            std::cout << s << std::endl;;
        }
        return true;
    } else {
        //std::cout << status.error_code() << ": " << status.error_message()
        //    << std::endl;
        return false;
    }
}
        
bool fbc::MessengerClient::Join(const std::string& username1,
        const std::string& username2) {
    Request request;
    //username1 is the person joining the chatroom
    request.set_username(username1);
    //username2 is the name of the room we're joining
    request.add_arguments(username2);

    Reply reply;

    ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    context.set_deadline(deadline);

    Status status = stub_->Join(&context, request, &reply);

    if(status.ok()){
        std::cout << reply.msg() << std::endl;
        return true;
    }
    else{
        //std::cout << status.error_code() << ": " << status.error_message()
        //    << std::endl;
        //std::cout << "RPC failed\n";
        return false;
    }
}
        
bool fbc::MessengerClient::Leave(const std::string& username1,
        const std::string& username2) {
    Request request;

    request.set_username(username1);
    request.add_arguments(username2);

    Reply reply;

    ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    context.set_deadline(deadline);

    Status status = stub_->Leave(&context, request, &reply);

    if(status.ok()){
        std::cout << reply.msg() << std::endl;
        return true;
    }
    else{
        //std::cout << status.error_code() << ": " << status.error_message()
        //    << std::endl;
        //std::cout << "RPC failed\n";
        return false;
    }
}
        
std::string fbc::MessengerClient::Login(const std::string& username) {
    Request request;

    request.set_username(username);

    Reply reply;

    ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    context.set_deadline(deadline);

    Status status = stub_->Login(&context, request, &reply);

    if(status.ok()){
        return reply.msg();
    }
    else{
        //std::cout << status.error_code() << ": " << status.error_message()
        //    << std::endl;
        return "RPC failed";
    }
}

bool fbc::MessengerClient::ServerAlive(const std::string& username) {
    Request request;

    request.set_username(username);

    Reply reply;

    ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(1);
    context.set_deadline(deadline);

    Status status = stub_->ServerAlive(&context, request, &reply);

    if (status.ok())
        return true;
    else
        return false;
}
        
void fbc::MessengerClient::Chat (const std::string& username) {
    ClientContext context;

    std::shared_ptr<ClientReaderWriter<Message, Message>> stream(
            stub_->Chat(&context));
    _stream = stream;
    
    if (first_chat) {

        std::thread writer([username]() {
            std::string input;
            Message m;

            m = MakeMessage2(username, "Set Stream");
            _stream->Write(m);
			first_chat = false;

            std::cout << "Enter chat messages: \n";

            while (getline(std::cin, input)) {
                m = MakeMessage2(username, input);

                while(connected != 2) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                _stream->Write(m);
                std::cout << "Enter chat messages: \n";
            }

            _stream->WritesDone();
        });
        writer.detach();
    } else {
        Message m = MakeMessage2(username, "Set Stream2");
        _stream->Write(m);
    }

    std::thread reader([username]() {
        Message m;
        while(_stream->Read(&m))
            std::cout << m.username() << " -- " << m.msg() << std::endl;
    });

    reader.join();
}

std::string fbc::MessengerClient::GetServerAddr(std::string username) {
    Request request;

    request.set_username(username);

    Reply reply;

    ClientContext context;
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(10);
    context.set_deadline(deadline);

    Status status = stub_->GetServerAddr(&context, request, &reply);

    if(status.ok()){
        return reply.msg();
    }
    else{
        //std::cout << status.error_code() << ": " << status.error_message()
        //    << std::endl;
        return "RPC failed";
    }
}


Message fbc::MessengerClient::MakeMessage(const std::string& username,
        const std::string& msg) {
    Message m;
    m.set_username(username);
    m.set_msg(msg);
    google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
    timestamp->set_seconds(time(NULL));
    timestamp->set_nanos(0);
    m.set_allocated_timestamp(timestamp);

    return m;
}
