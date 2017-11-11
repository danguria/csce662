The Chat Room Client(fbc) {#client}
===============================
For the client side, There are two classes creating individual stub in order to interact
with server. First stub is for the purpose of calling LIST, JOIN, LEAVE, CHAT command.
Second one is designed for listening another user who is posting to the joined room in
CHAT mode. In the first class, each functionality has its associated function call in order
to communicate with the server. These are all gRPC calls based on the protocol buffer
as described in the .proto file.


Clients, by default, have its own chat room ‘<username> when a client first implements
the application as planned. That is, ./fbc <hostname> <port_comm> <port_chat> <username>. It is
executed by calling the joinRomm function which has a special feature. The function
can create a room and then join the room by passing 0 value to second parameter.
Users can see the existing rooms list and the rooms a user already visited by calling
showList function. In the function call, there are two rpc calls. The ‘GetAllChatRooms’
rpc call is invoked for retrieving all created rooms. The ‘GetJoinedChatRooms’ rpc call
retrieves the rooms list that a user already entered before. Both of two rpc calls
exchange data by passing User message and getting Chatroom stream data messages.
When users want to join a room, the function ‘joinRoom’ is in charge of that
functionality. It invokes the related rpc call ‘JoinChatRoom’ by sending request message
and in turn receiving reply message. Likewise, when users want to leave a room, the
function ‘levaeRoom’ takes charge of that. It also invokes the rpc call ‘LeaveChatRoom’
giving and receiving the same message with the join rpc call.


Clients have access to the chat mode by invoking the function ‘chatInRoom’. Once a
user enters its room, previous and recent 20 messages will be retrieved from the server
by invoking ShowChats rpc call. Then, the user can post messages or get messages
from others who joined the user’s room. For this, two threads are created. First thread is
in charge of List, Join, Leave, partial Chat feature. The second thread is designed to
wait for other’s messages by assigning another stub that has its own port number.
