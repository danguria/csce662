/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Will Adams and Nicholas Jackson
   CSCE 438 Section 500*/

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#include <grpc++/grpc++.h>

#include "chatroom.grpc.pb.h"
#include "fbc/messenger_client.h"
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using hw2::Message;
using hw2::ListReply;
using hw2::Request;
using hw2::Reply;
using hw2::ChatRoomService;

fbc::MessengerClient *_messenger = NULL;
std::string _addr;
extern int connected;

void ReConnectToServer(std::string username);
int parse_input(std::string username, std::string input);
void RunServer(std::string addr, std::string username);
std::string GetServerAddr(std::string masterAddr, std::string username);

int main(int argc, char** argv) {
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).

    std::string hostname = "localhost";
    std::string username = "default";
    std::string port = "3010";
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:u:p:")) != -1){
        switch(opt) {
            case 'h':
                hostname = optarg;break;
            case 'u':
                username = optarg;break;
            case 'p':
                port = optarg;break;
            default: 
                std::cerr << "Invalid Command Line Argument\n";
        }
    }

    _addr = hostname + ":" + port;
    std::string serverAddr = GetServerAddr(_addr, username);
    RunServer(serverAddr, username);
    return 0;
}


//Parses user input while the client is in Command Mode
//Returns 0 if an invalid command was entered
//Returns 1 when the user has switched to Chat Mode
int parse_input(std::string username, std::string input) {
    //Splits the input on spaces, since it is of the form: COMMAND <TARGET_USER>
    std::size_t index = input.find_first_of(" ");
    if(index!=std::string::npos){
        std::string cmd = input.substr(0, index);
        if(input.length()==index+1){
            std::cout << "Invalid Input -- No Arguments Given\n";
            return 0;
        }
        std::string argument = input.substr(index+1, (input.length()-index));
        if(cmd == "JOIN"){
            while (!_messenger->Join(username, argument)) ReConnectToServer(username);
        }
        else if(cmd == "LEAVE")
            while (!_messenger->Leave(username, argument)) ReConnectToServer(username);
        else{
            std::cout << "Invalid Command\n";
            return 0;   
        }
    }
    else{
        if(input == "LIST"){
            while (!_messenger->List(username)) ReConnectToServer(username);
        }
        else if(input == "CHAT"){
            //Switch to chat mode
            return 1;
        }
        else{
            std::cout << "Invalid Command\n";
            return 0;   
        }
    }
    return 0;   
}

void RunServer(std::string addr, std::string username) {

    //Create the messenger client with the login info
    _messenger = new fbc::MessengerClient(grpc::CreateChannel(
                _addr, grpc::InsecureChannelCredentials())); 
    //Call the login stub function
    std::string response = _messenger->Login(username);
    //If the username already exists, exit the client
    if(response == "Invalid Username"){
        std::cout << "Invalid Username -- please log in with a different username \n";
        return;
    }
    else{
        std::cout << response << std::endl;

        std::cout << "Enter commands: \n";
        std::string input;
        //While loop that parses all of the command input
        while(getline(std::cin, input)){
            //If we have switched to chat mode, parse_input returns 1
            if(parse_input(username, input) == 1)
                break;
        }
        //Once chat mode is enabled, call Chat stub function and read input
        std::thread watchdog ([username]() {

            while(1) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (!_messenger->ServerAlive(username)) {
                    connected = 0; // disconnected
                    ReConnectToServer(username);
                }
                connected = 2;
            }
        });
        watchdog.detach();
        while (1) {
            _messenger->Chat(username);
			//ReConnectToServer(username);
        }
    }
}

std::string GetServerAddr(std::string masterAddr, std::string username) {

    fbc::MessengerClient *messenger = new fbc::MessengerClient(grpc::CreateChannel(
                masterAddr, grpc::InsecureChannelCredentials())); 

    std::string addr = messenger->GetServerAddr(username);

    delete messenger;

    return addr;
}

void ReConnectToServer(std::string username) {

		//std::cout << "Disconnected,  Retryig to connect.." << std::endl;
    std::string addr = GetServerAddr(_addr, username);
		//std::cout << "Please wait for 30 secs to be connected..." << std::endl;

    if (_messenger != NULL) delete _messenger;
    _messenger = new fbc::MessengerClient(grpc::CreateChannel(
                _addr, grpc::InsecureChannelCredentials())); 

    std::string response = _messenger->Login(username);
}
