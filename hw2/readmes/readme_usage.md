Usage {#usage}
============
1. Directory
------------
This project supplied as a git repository on github - https://github.tamu.edu/ksungkeun84/csce662.git
The github is the private repository if you want to join, please let me know.

`./src/protos`      : Contains all the protocol buffer files and its generated .cc files.

`./src/fbsd`        : Contains all the server source .cc files.

`./src/fbc`         : Contains all the client source .cc files.

`./include`         : Contains the program header include files.

`./builds`          : Contains intermediate files such as .o files.

`./doxygen-cfg.txt` : File to generate a documentation using __doxygen__.

`./README.md`       : This readme text file - which is also processed by __doxygen__.

`./readms`          : Contains all the documentations to be converted to pdf.

`./makefile`        : master makefile. 

2. Building the Chat Room Service
------------------------------
Run `make` from the root of the installation. This will build both the client and server executable files and deliver these into the `./bin/` directory and intermediate file such as object files will be located in `./builds` directory.

3. Using the Chat Room Service
---------------------------
__- FBSD (Server)__:

Run `./bin/fbsd PORT_COMM PORT_CHAT`. Here, `PORT_COMM` means the port number for command mode communication and `PORT_CHAT` means the port number for the chat mode communication. You can choose any available port number as the command line arguments. For example, we used ports `50051` and `50052` for the communication.

    ./bin/fbsd 50051 50052

If your target machine already use this port, you can change it with available port. Onece you succeed in running server program, you are ready to run the client program.

__- FBC  (Client)__:

Run `./bin/fbc IP PORT_COMM PORT_CHAT USER`. Here, IP is the ip address of the server, PORT_COMM is the port number for the command mode communication, and PORT_CHAT is the port number for the chat mode communication. The USER is the user name that you want to join. For example, we used `localhost` as IP, `50051` and `50052` as the ports number, and `Bob` as the user name..

    ./bin/fbc localhost 50051 50052 Bob

Once you succedd to run client program, you are ready to send the command to the server side.
