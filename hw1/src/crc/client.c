#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "utils.h"

int connCreation (char *host, int port);
int deliverer (char *roomName, int sockfd, char *command);
void openmenu (int* num);
	
int main(int argc, char *argv[]) {
	int sockfd, portno, checker, n;
	int numberSelection = 0;

	char buffer[MAX_DATA];
	char userInput[MAX_USER_INPUT];

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	
	portno = atoi(argv[2]);
	sockfd = connCreation(argv[1], portno);

	//---after connection

	bzero(buffer,256);
	bzero(userInput,20);

	n = read(sockfd,buffer,255);
				
	if (n < 0) {
		perror("ERROR reading from socket");
		exit(1);
	}
	printf("%s\n",buffer);

	
	do{
	openmenu(&numberSelection);

	bzero(buffer,256);
	bzero(userInput,20);

        
	if(numberSelection == 1)
	{
		printf("-----Write the chat-room name you want to create.-----\n");
		do
		{
			scanf("%s",userInput);
			checker = deliverer(userInput, sockfd, "CREATE:");	
			if(checker < 0)
				printf("Try again for creating new chatroom!\n");
		}while(checker < 0);
	
		printf("The room %s (portnumber %d) is successfully created!\n", userInput, checker);
	}
	
	else if(numberSelection == 2)
	{
		system("clear");
		printf("Write the chat-room name you want to delete : \n");

		do
		{
			scanf("%s",userInput);
			checker = deliverer(userInput, sockfd, "DELETE:");
			if(checker < 0)
				printf("Try again for deleting a chatroom!\n");		
		}while(checker < 0);
	
		printf("The room %s is successfully deleted!\n", userInput); 
	}		

	else if(numberSelection == 3)
	{
		system("clear");
		printf("Write the chat-room name you want to enter : \n");

		do
		{
			scanf("%s",userInput);
			checker = deliverer(userInput, sockfd, "JOIN:");	
			if(checker < 0)
				printf("Try again for joining a chatroom!\n");		
		}while(checker < 0);

		close(sockfd); 	// close current Socket connection.
		sockfd = connCreation(argv[1], checker); //new creation of Socket connection based on the newly created port.
	
		printf("You joined the room %s(port:%d) successfully!\n", userInput, checker); 
		

		n = read(sockfd,buffer,255);
					
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
		printf("%s\n",buffer);

		bzero(buffer,256);
		
		fd_set master;
		fd_set readfds;
		FD_ZERO(&master);
		FD_ZERO(&readfds);
		
		FD_SET(sockfd, &master);
		FD_SET(STDIN_FILENO, &master);

		int disconnected = 0;

		while (1) {
			printf("enter your msg: ");

			readfds = master;
			if(select(FD_SETSIZE, &readfds, NULL, NULL, NULL) == -1)
			{
				perror("select");
				exit(1);
			}
			
			int i = 0;

			for (i = 0; i <= FD_SETSIZE; i++){

				if (FD_ISSET(i, &readfds)) {
					if(i == STDIN_FILENO){
						fgets(buffer, 256, stdin);
						if(send(sockfd, buffer, strlen(buffer), 0) == -1)
						perror("send"); // incoming QUIT command processing.
					} else if (i == sockfd){
					  	n=recv(sockfd, buffer, 256-1, 0);
						if (n == 0 ) {
							// connection disconected
							printf("debug1\n");
							close(sockfd);
							disconnected = 1;
							break;
						} else if (n < 0) {
							perror("recv");
							exit(1);
						} else {
							printf("\n you got message : ");
							//parsing needed.
							buffer[n] = '\0';
							printf("%s\n",buffer);
						}
					}
				}
			}	
		bzero(buffer,256);

		if (disconnected) break;
		}
	close(sockfd);
	}		

	else if(numberSelection == 4){
		deliverer(NULL, sockfd, "LIST");	
	}

	if (numberSelection <= 0 ||  numberSelection >= 5) {
		printf("unkown command, try again\n");
		char ch = getchar();
		while ( ch != '\n' || ch == EOF) {
			ch = getchar();
		}
	}

	}while(1);
}

int connCreation (char *host, int port) {
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	// portno = atoi(port);

	//---socket creation
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	server = gethostbyname(host);

	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	//---host and port handling
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

        //---connection establishment
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR connecting");
		exit(1);
	}

	return sockfd;
} 
	
int deliverer (char *roomName, int sockfd, char *command)
{
	int n,aux_id;
	
	char buffer[256];
	char *identifier;
	
	
	bzero(buffer,256);

	if(strcmp(command, "LIST")==0)
	{
		n = write(sockfd,"LIST",5);

		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}
	
		n = read(sockfd,buffer,255);
				
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}
	system("clear");
	printf("The chat-room list is below.\n");
	printf("%s \n", buffer); // parsing is needed.
	
	}

	else
	{
		strcpy(buffer, command);
		strcat(buffer, roomName);
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
		}

		n = read(sockfd,buffer,255);

		identifier = strtok(buffer,":");	
		
		if (n < 0) {
			perror("ERROR reading from socket");
			exit(1);
		}

		if (strcmp(identifier,"DENIDED")==0)
		{
			return -1;			
		}
		else if(strcmp(identifier,"SUCCESS")==0)
		{
			aux_id = atoi(strtok(NULL,":"));
			return aux_id; 
		}
	}

}
void openmenu (int* num)
{
	printf("----- Type the number to excute the job. -----\n");
	printf("1. Create a chat-room.\n");
	printf("2. Delete a chat-room.\n");
	printf("3. Join a chat-room.\n");
	printf("4. See chat-rooms list.\n");
	printf("Press Ctrl+C to exit\n");
	printf("----------------------------------------------\n");
	scanf("%d",num);
}
