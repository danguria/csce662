The Chat Room Client(crc) {#client}
===============================
For the client program, four functions in total are constructed including the main function. In general, the main function handles all procedures and takes control over other functions as usual. Other three functions are invoked when needed. The connCreation function is utilized to implement socket creation part, i.e. calling the already defined socket() system call, and addressing hostname, port, etc. The deliverer function is designed to interact with the counterpart server. The openmenu function is for the purpose of user-interface. Once a user connects to the server by typing “client hostname port” in the command line, the defined user-interface appears.

Clients can execute desired operations by writing related numbers in the user-interface. Creating a chat room (#1), Deleting a chat room(#2), Joining a chat room(#3). If and If-else statements handles this separation in the code.

Users can open a chat room by placing a room name. If the room name already exists, making the room is denied and subsequently “try another room name message” will be present. Otherwise, the room name would be safely saved in the cooperating server. Optionally, users can type “LIST” as a room name afterwards the previously created rooms list will be shown. 

Users can delete an existing chat room by writing the chat room name. If the room name is incorrect or isn’t in existence, the deny message will come and then the re-try message happens. Otherwise, the room name typed by the user will be successfully deleted. In the meantime, if there are users who communicates each other in the deleting room they have the warning saying that the room is in the process of removing. Shortly, the room disappears resulting in all chatters in the room dismiss.

Users also can join a chat room by entering the room name. If there are no room names, likewise retry is needed. If the chat room exists, users can join the room. Joining a room step internally needs disconnecting the already accessed server and reconnecting the server again with another port assigned by the server. It is done by calling the function connCreation(). Users can freely talk and discuss in the joined chat room. However, from the perspective of machines, the program sometimes receives inputs from command line and other times receives the socket. For handling this, select() system call is used. if a user want to be out of the room, it can be done by simply typing “QUIT” on the command line.

Each client should have the socket client program for interacting the server and communicating with others. Whenever users try to send requests such as connecting the server, creating rooms, etc, the server automatically takes charge for those regardless of the number of users.
