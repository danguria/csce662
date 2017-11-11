/*!
 * \file       utils.h
 * \brief      Utility functions for chatting application
 */
#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <errno.h>
#include <ctype.h>


/** maximbum data for buffer used by send, recv/send system call*/
#define MAX_DATA 1024
/** maximum size of room name */
#define MAX_ROOM_NAME 128
/** maximum size of user input */
#define MAX_USER_INPUT 128  // it can contain romm name

/** For debugging purpose */
/** macros to handle perror gracefully */
#define handle_error_en(en, msg) \
                   do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

/** macros to handle perror gracefully */
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

/** tag string for debugging purpose */
#define DEBUG_TAG "crsd"

/*! used as argument to pthread */
struct thread_arg {
    pthread_t thread_id;  /**< ID returned by pthread_create() */
    int thread_num;       /**< Application-defined thread */
    int listener;         /**< listening socket descriptor for chat_handler; */
    char port[8];         /**< port for server socket, used by command_handler */
};

/** @brief convert to lowercase string
    @param str target string to be lowercased
    @param n
    */
void tolowercase(char *str, int n);

/** @brief print information about ip address
  @param ai address info structure
  */
void print_ipinfo(struct addrinfo *ai);

/** @brief get socket address IPv4 or IPv6:
    Utility function for getting sockaddr structure for IPv4 or IPv6

    @param sa sockaddr which is commonly use for both IPv4 and IPv6
    @return one of pointer for sockaddr_in or sockaddr_in6
  */
void* get_in_addr(struct sockaddr *sa);

#endif
