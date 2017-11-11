HW3: An Fault-tolerant Chat Room Service
=========================================

TEAM MEMBER
=========================================
- Sungkeun Kim (UIN: 325003839) : in charge of ther server
- Jaeguen Byoun (UIN: 625004109) : in charge of the client

BUILDING THE CHAT ROOM SERVICE
=========================================
Run `make` from the root of the installation. This will build both the client and server executable files and deliver these into the `./bin/` directory and intermediate file such as object files will be located in `./builds` directory.

LAUNCHING THE SERVER
======================

Configure server-cfg.txt
---------------------------
Replace "IPX" to real ip address. Make sure that three machine have the exactly same server-cfg.txt. If configured ports are in use, you can change it to unused port.

0:IP1:4010:3010
1:IP1:4011:3010
2:IP1:4012:3010
3:IP2:4013:3010
4:IP2:4014:3010
5:IP2:4015:3010
6:IP3:4016:3010
7:IP3:4017:3010
8:IP3:4018:3010
9:IP3:4019:3010

Run start-up scrip
---------------------------
Run start-up script using below command:

	./run.py machine-ip

This command read server-cfg.txt and launch the servers. 


USING A CLIENT PROGRAM
=========================================
run command below:
	
	./bin/fbc -h host -p 3010 -u username
