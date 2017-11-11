/*!
 * \file       command.h
 * \brief      Functions for command mode
 */
#ifndef COMMAND_H
#define COMMAND_H

#include "utils.h"
#include "chat.h"

/** Enumberation for command */
enum Command { CREATE, JOIN, DELETE, LIST, UNKNOWN };

/*! Command structure for command mode */
struct command{
    int fd; /**< slave fd of sender(client) */
    int type; /**< type of command one of the Command enumeration */
    char room_name[MAX_ROOM_NAME]; /**< room name related with the command */
};

/** @brief parse command sent from the clients
  
  parse command and save these to command struct.
  possible command lists:.
    CREATE:name,
    DELETE:name,
    JOIN:namei,
    and LIST.

    @param fd       slave fd for command mode
    @param buf_recv command stored here
    @param nbytes   number of buf_recv
    @param comm     command structure that stores the result of parsing
     
*/
void parse_command(int fd, char *buf_recv, int nbytes, struct command *comm);

/** @brief main function of command mode
  This function is invoked by newly created threads for command mode. Each connection for command mode has their own thread.
  @param arg struct thread_arg will be passed as parameter
  */
void* command_handler(void *arg);


/** @brief broadcast a message to a specified chat room
  This function is used for warnning message from server to specific chat room.
  For example, when there is DELETE request for that chat room, then server broadcast  warning message to clients who are talking in that chat room.

  @param chat chat information to be received a message
  @param msg message to the chat room.
  */
void broadcast_to(struct chatinfo* chat, char* msg);
#endif
