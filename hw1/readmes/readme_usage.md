Usage {#usage}
============
1. Directory
------------
This project supplied as a git repository on github - https://github.tamu.edu/ksungkeun84/csce662.git

`./src` : Contains all the program source .c files.

`./include`: Contains the program header include files.

`./builds`  : Contains intermediate files such as .o files.

`./doxygen-cfg.txt` : File to generate a documentation using __doxygen__.

`./README.md` : This readme text file - which is also processed by __doxygen__.

`./TODO.md` : Toto lists

`./readms` : Contains all the documentations to be converted to pdf.

`./makefile`  : master makefile. 

2. Building the Chat Room Service
------------------------------
Run `make` from the root of the installation. This will build both the client and server executable files and deliver these into the `./bin/` directory and intermediate file such as object files will be located in `./builds` directory.

3. Using the Chat Room Service
---------------------------
__- CRSD (Server)__:

Run `./bin/crsd PORT`. Here, `PORT` means the port number for command mode communication. You can choose any available port number as the command line arguments. For example, we used 3490 as the command mode communication.

    ./bin/crsd 3490

If your target machine already use this port, you can change it with available port. Onece you succeed in running server program, you are ready to run the client program.

__- CRC  (Client)__:

Run `./bin/crc IP PORT`. Here, IP is the ip address of the server and PORT is the port number that the server is using for the command mode communication. For example, we used localhost as IP and 3490 as the port number.

    ./bin/crc localhost 3490

Once you succedd to run client program, you are ready to send the command to the server side.
