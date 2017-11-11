The Chat Room Server(crsd) {#server}
=================================

Key Characteristics
-------------------
The following is the key characteristics of the chat room service.

1. Multithreaded program using both *POSIX* pthread library and *select* system call.
2. Dynamic port allocation for the chatting room.

1. Multithreaded Program
------------------------
Server side program is using both *the pthread* library and *the select* system call. For the command mode, the server has the only one thread to handle connection requests from the clients. In this thread, *select* system call is used to handle both the connection and command request. For the chat mode, each chat romm created by the user's request has its own thread. Each thread handles both the connection requests and the chat messages using *select* system call.

![Server Structure](server_structure.jpg)
@image latex server_structure.jpg

2. Dynamic port allocation
--------------------------
For the command mode, we have fixed port number that is given by command line arguments. However, because each chat romm has to have distinct port number, server finds available port dynamically ranges from the registered ports(1024 ~ 49151) and the dynamic ports(49152 ~ 65535) for the chat room. Therefore, the number of available chat room can vary. For this reason, the number of the chat romms doesn't fixed, so the information of chat room is implemented by the linked list data structure.

3. Limitation
-------------
Bothe SRSD and CRC have fixed size for the chat room and message buffer. The maximum size of chat room is 128 bytes and message buffer is 1024 bytes. If user make larger message than the maximum size, we cannot guarantee the expected behavior.
