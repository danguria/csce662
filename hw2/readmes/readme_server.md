The Chat Room Server(fbsd) {#server}
=================================

Key Characteristics
-------------------
The following is the key characteristics of the chat room service.

1. Two Synchronous gRPC services for the communication
2. Using Simple file format for saving all the contents of the chat room..

1. Two Synchronous gRPC services
--------------------------------
The Server and Client are communicating using the two *Synchronous* gRPC services so that the client can post their message and update their chat room at the same time. In other words, while the client waits for the new updates, synchronous gRPC is blocked until server replies. In that case, the client cannot post their own messages to server. To solve this problem, we use the additional gRPC service to post messages. Because of this, we also use two ports for the communication.

![gRPC Services](server_structure.jpg)
@image latex server_structure.jpg

2. Simple file format for the chatroom service
----------------------------------------------
We created a simple file format for saving the contents of the runtime changes. This is because we want our program independent from other libraries for now. For example, we can use C++ boost library to serialize and deserialize every contents of the chat room, but we don't know whether test environment can use this kind of libraries.

File format is pretty simple and looks like as follow:

![File Format](file_format.png)
@image latex file_format.png

There can be multiple ChatRooms and be multiple Participants and Chats inside the ChatRoom. When the server starts, it reads `data.dat` file and re-load chatromms and its contents. Whenever new contents appears, the server saves these to the file.
