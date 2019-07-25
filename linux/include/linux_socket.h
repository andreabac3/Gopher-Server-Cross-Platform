//
// Created by valerioneri on 6/13/19.
//

#ifndef GOPHER_LINUX_SOCKET_H
#define GOPHER_LINUX_SOCKET_H

#include <stdbool.h>
#include "definitions.h"


int linux_socket(struct Configs* configs, bool first_time);
void * handle_request(void * args);
void* handle_request_thread(void *params);
int end_server(int fd);
#endif //GOPHER_LINUX_SOCKET_H
