//
// Created by valerioneri on 6/13/19.
//

#ifndef GOPHER_LINUX_SOCKET_H
#define GOPHER_LINUX_SOCKET_H

#include "definitions.h"

int blackListFile(char *baseDir, char *pathFile, char *black_listed_file);
int linux_socket(struct Configs* configs);
void * handle_request(void * args);
void* handle_request_thread(void *params);
int end_server(int fd);
void close_mutex();
void start_mutex();
void * l_sendFile(void *send_args);
void * linux_sendFile(void *send_args);
int start_server(unsigned int port, int queue_size);
#endif //GOPHER_LINUX_SOCKET_H
