#include "utils.h"
#include "chat.h"

// should be synchronized with lock_chatinfo
struct chatinfo *g_chatinfo = NULL;
pthread_mutex_t lock_chatinfo;

static void* chat_handler(void *arg) {

    struct thread_arg *targ = arg;

    fd_set master;   // master file descriptor list
    fd_set readfds;  // temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number

    int newfd;       // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf_recv[256];
    char buf_send[256];
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    FD_ZERO(&master);   // clear the master and temp sets
    FD_ZERO(&readfds);

    // add the listener to the master set
    FD_SET(targ->listener, &master);

    // keep track of the biggest file descriptor
    fdmax = targ->listener; // so far, it's this one

    // main loop
	int i;
    for(;;) {

        readfds = master;
        if (select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {

            if (FD_ISSET(i, &readfds)) { // we got one!!

                // handle new connections
                if (i == targ->listener) {
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(targ->listener, (struct sockaddr*)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("crsd: new connection from %s on socket %d for the chatting mode\n",
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
                        int delete = 0;
                        if (strcmp(buf_recv, "admin-delete") == 0)  {
                            memset(buf_recv, 0, sizeof(buf_recv));
                            char msg[128] = "Warnning: the chatting room is going to be closed...";
                            sprintf(buf_recv, "%s", msg);
                            delete = 1;
                        }
                        // we got a chat from a client
						int j;
                        for (j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != targ->listener && j != i) {
                                    memset(buf_send, 0, sizeof(buf_send));
                                    sprintf(buf_send, "%d: %s", i, buf_recv);
                                    if (send(j, buf_send, strlen(buf_send), 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }

                        if (delete) {
                            for (j = 0; j <= fdmax; j++) {
                                // send to everyone!
                                if (FD_ISSET(j, &master)) {
                                    // except the listener and ourselves
                                    if (j != targ->listener) {
                                        printf("closing slave fd: %d\n", j);
                                        close(j);
                                    }
                                }
                            }
                            // close master fd last
                            printf("closing master fd: %d\n", targ->listener);
                            close(targ->listener);
                            free(targ);
                            return "deleted";
                        }
                    }
                }
            }
        }
    }

	int j;
    for (j = 0; j <= fdmax; j++) {
        // send to everyone!
        if (FD_ISSET(j, &master)) {
            // except the listener and ourselves
            if (j != targ->listener) {
                printf("closing slave fd: %d\n", j);
                close(j);
            }
        }
    }
    // close master fd last
    printf("closing master fd: %d\n", targ->listener);
    close(targ->listener);
    free(targ);
    return "unkown termination";
}

static void create_new_conn(struct chatinfo* chatinfo) {

    struct addrinfo hints, *ai, *p;

    // get us a socket and bind it
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int listener;    // listening socket descriptor
    int port_lookup = 1024, port = -1;
    char sport[8];
    int ret;


    for (; port_lookup < 65535; port_lookup++) {

        sprintf(sport, "%d", port_lookup);
        ret = getaddrinfo(NULL, sport, &hints, &ai);
        if (ret != 0) {
            fprintf(stderr, "crsd: %s\n", gai_strerror(ret));
            exit(1);
        }

        //print_ipinfo(ai);
        int yes = 1;     // for setsockopt() SO_REUSEADDR, below

        for (p = ai; p != NULL; p = p->ai_next) {
            listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (listener < 0) {
                continue;
            }

            // lose the pesky "address already in use" error message
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

            if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
                close(listener);
                continue;
            }

            port = port_lookup;
            break;
        }

        freeaddrinfo(ai);
        if (port != -1) break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL || port == -1) {
        fprintf(stderr, "crsd: failed to bind\n");
        exit(2);
    }

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // save port to chatinfo
    chatinfo->port = port;

    // create thread for the chat from the clients
    struct thread_arg *targ = (struct thread_arg*)malloc(sizeof(struct thread_arg));
    targ->thread_num = 1;
    targ->listener = listener;

    ret = pthread_create(&(targ->thread_id), NULL, chat_handler, targ);
    if (ret != 0) {
        handle_error_en(ret, "pthread_create");
    }
}

void init_chat() {
    if (pthread_mutex_init(&lock_chatinfo, NULL) != 0) {
        printf("\n mutex init failed\n");
        exit(1);
    }
}

void final_chat() {
    pthread_mutex_destroy(&lock_chatinfo);
}

/** 
  * find a chatinfo structure matching with given name
  * param name room name for searching
  * return chatinfo structure if it exists
  */
struct chatinfo* get_chat(char *name) {
    pthread_mutex_lock(&lock_chatinfo);
    struct chatinfo *p;

    for (p = g_chatinfo; p != NULL; p = p->next) {
        if (strcmp(p->room_name, name) == 0) {
            pthread_mutex_unlock(&lock_chatinfo);
            return p;
        }
    }
    pthread_mutex_unlock(&lock_chatinfo);
    return NULL;
}

struct chatinfo* add_newchat(char* name) {
    if (get_chat(name) != NULL) return NULL;
    struct chatinfo *chat = (struct chatinfo*)malloc(sizeof(struct chatinfo));
    strncpy(chat->room_name, name, sizeof(chat->room_name));
    pthread_mutex_lock(&lock_chatinfo);
    if (g_chatinfo == NULL) {
        chat->next = NULL;
    } else {
        chat->next = g_chatinfo;
    }
    g_chatinfo = chat;
    pthread_mutex_unlock(&lock_chatinfo);

    create_new_conn(chat); // port will be set
    return chat;
}

struct chatinfo*  delete_chat(char* name) {
    pthread_mutex_lock(&lock_chatinfo);

    struct chatinfo* prev, *cur;
    prev = cur = g_chatinfo;
    while (cur != NULL) {
        if (strcmp(cur->room_name, name) == 0) break;
        prev = cur;
        cur = cur->next;
    }

    // there isn't chat with input name
    if (cur == NULL) {
        pthread_mutex_unlock(&lock_chatinfo);
        return NULL;
    }

    // deteting node is head
    if (prev == cur) {
        // there is only one chat make g_chatinfo NULL
        if (cur->next == NULL)
            g_chatinfo = NULL;
        else
            g_chatinfo = cur->next;
    } else {
        prev->next = cur->next;
    }

    pthread_mutex_unlock(&lock_chatinfo);
    return cur;
}

void get_list(char *buf) {
    pthread_mutex_lock(&lock_chatinfo);
    struct chatinfo *p;
    char sport[8];
    for (p = g_chatinfo; p != NULL; p = p->next) {
        strcat(buf, p->room_name);
        strcat(buf, ":");
        sprintf(sport, "%d", p->port);
        strcat(buf, sport);
        strcat(buf, ",");
    }
    int len = strlen(buf);
    if (len > 0)
        buf[strlen(buf) - 1] = '\0';
	else
		strcpy(buf, "empty");
    pthread_mutex_unlock(&lock_chatinfo);
}
