/*!
 * \file       chat.h
 * \brief      Functions for chatting mode
 */
#ifndef CHAT_H
#define CHAT_H

#include "utils.h"
#include "chat.h"

/*! chatting room structure.
  This structure contains chatting room information.
  Port number is dynamically allocated between registered 
  port(1024 ~ 49151) and dynamic port(49152 ~ 65535).
  */
struct chatinfo {
    char room_name[MAX_ROOM_NAME]; /**< name of a chat room*/
    int port;                      /**< port number used for this chatroom */
    struct chatinfo *next;         /**< next chatting room inforation, chatinfo is managed by linked list */
};

/** @brief initialize chat environments
  */
void init_chat();

/** @brief finalize chat environments
  */
void final_chat();

/** @brief returns chatinfo

  find a chatinfo structure matching with given name
  Data zeroed, request flags for counters/comparators set to
  access all.

  @param name room name for searching
  @return chatinfo structure if it exists

*/
struct chatinfo* get_chat(char *name);

/** @brief create new chat room with given name
  In this function, new chat room will be created with given name and new thread for chat mode will be created and listening incomming request.
    @param name name of chat room for creation
    @return struct chatinfo newly created return if there isn't chat room with given name, otherwise NULL return
  */
struct chatinfo* add_newchat(char *name);

/** @brief delete existing chat room
    This function removes chatinfo from the g_chatinfo

    @param name name of chat room for deletion
    @return struct chatinfo deleted from g_chatinfo if there exists chat room with given name, otherwise NULL return
  */
struct chatinfo*  delete_chat(char *name);

/** @brief returns all the chatinfo in the g_chatinfo
  This function returns all the created chatinfo with the pattern as follow:
NAME:port, NAME:port, ..., NAME:port
  @param buf this parameter is used to store all the chatinfo
  */
void get_list(char *buf);

extern struct chatinfo *g_chatinfo;
extern pthread_mutex_t lock_chatinfo;
#endif
