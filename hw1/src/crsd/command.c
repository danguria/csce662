#include "utils.h"
#include "command.h"
#include "chat.h"

void parse_command(int fd, char *buf_recv, int nbytes, struct command *comm) {
    char *pch;
    comm->fd = fd;

    tolowercase(buf_recv, nbytes);
    pch = strtok(buf_recv, ":");
    if (strcmp(pch, "create") == 0)
        comm->type = CREATE;
    else if (strcmp(pch, "join") == 0)
        comm->type = JOIN;
    else if (strcmp(pch, "delete") == 0)
        comm->type = DELETE;
    else if (strcmp(pch, "list") == 0) {
        comm->type = LIST;
        return;
    }
    else
        comm->type = UNKNOWN;

    pch = strtok(NULL, ":");
    if (pch != NULL) strncpy(comm->room_name, pch, MAX_ROOM_NAME);
    else comm->type = UNKNOWN;
}

void* command_handler(void *arg) {

    struct thread_arg *targ = arg;

    fd_set master;   // master file descriptor list
    fd_set readfds;  // temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number

    int listener;    // listening socket descriptor
    int newfd;       // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf_recv[MAX_DATA];   // buffer for client data
    char buf_send[MAX_DATA];   // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes = 1;     // for setsockopt() SO_REUSEADDR, below
    int ret;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);   // clear the master and temp sets
    FD_ZERO(&readfds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    ret = getaddrinfo(NULL, targ->port, &hints, &ai);
    if (ret != 0) {
        fprintf(stderr, "crsd: %s\n", gai_strerror(ret));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) continue;

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        ret = bind(listener, p->ai_addr, p->ai_addrlen);
        if (ret < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "crsd: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    ret = listen(listener, 10);
    if (ret == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
	int i;
    for(;;) {

        readfds = master;
        ret = select(fdmax + 1, &readfds, NULL, NULL, NULL);
        if (ret == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {

            if (FD_ISSET(i, &readfds)) { // we got one!!

                // handle new connections
                if (i == listener) {
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("crsd: new connection from %s on socket %d for the command mode\n",
                                inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN),
                                newfd);
                        char greetings[126];
                        sprintf(greetings, "Hello! You have been successfully connected to server with fd %d", newfd);
                        if (send(newfd, greetings, strlen(greetings), 0) == -1)
                            perror("send");
                    }

                // handle data from a client
                } else {
                    memset(buf_recv, 0, sizeof(buf_recv));
                    nbytes = recv(i, buf_recv, sizeof(buf_recv), 0);
                    if (nbytes <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("crsd: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        struct command comm;
                        memset(&comm, 0, sizeof(struct command));
                        printf("crsd: got command from %d - %s\n", i, buf_recv);
                        parse_command(i, buf_recv, nbytes, &comm);
                        printf("crsd: parsed as:\n");
                        printf("      fd: %d\n", comm.fd);
                        printf("      type: %d\n", comm.type);
                        printf("      room_name: %s\n", comm.room_name);
                        memset(buf_send, 0, sizeof(buf_send));
                        struct chatinfo *cinfo = NULL;
                        switch (comm.type) {
                            case CREATE:
                                    cinfo = add_newchat(comm.room_name);
                                    if (cinfo != NULL) sprintf(buf_send, "SUCCESS:%d", cinfo->port);
                                    else strcpy(buf_send, "DENIEDED");
                                    break;
                            case JOIN:
                                    cinfo = get_chat(comm.room_name);
                                    if (cinfo != NULL) sprintf(buf_send, "SUCCESS:%d", cinfo->port);
                                    else strcpy(buf_send, "DENIEDED");
                                    break;
                            case DELETE:
                                    cinfo = delete_chat(comm.room_name);
                                    if (cinfo != NULL) {
                                        broadcast_to(cinfo, "admin-delete");
                                        free(cinfo);
                                        strcpy(buf_send, "SUCCESS");
                                    } else
                                        strcpy(buf_send, "DENIEDED");
                                    break;
                            case LIST:
									printf("crsd: send message to client %d - %s\n", i, buf_send);
                                    get_list(buf_send);
                                    break;
                            default:
                                    strcpy(buf_send, "UNKNOWN COMMAND");
                        }
                        if (send(i, buf_send, strlen(buf_send), 0) == -1) {
                            perror("send");
                        }
                    }
                }
            }
        }
    }
}

void broadcast_to(struct chatinfo *chat, char *msg) {
    int sockfd, numbytes;
    char buf[MAX_DATA];
    struct addrinfo hints, *servinfo, *p;
    int ret;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char sport[8];
    sprintf(sport, "%d", chat->port);

    if ((ret = getaddrinfo(NULL, sport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(1);
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("crsd: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("crsd: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "crsd: failed to connect\n");
        exit(1);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
    printf("crsd: connecting to %s for the broadcasting..\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAX_DATA - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("crsd: connected to chat room\n");

    // broad cast to other member
    if (send(sockfd, msg, strlen(msg), 0) == -1) perror("send");

    close(sockfd);
}
