//
// Created by andrea on 14-Jun-19.
//

#ifndef GOPHERWIN_WINDOWS_SOCKET_H
#define GOPHERWIN_WINDOWS_SOCKET_H

//int windows_socket_runner();

#include "definitions.h"
int windows_socket_runner(struct Configs *configs);
int w_sendFile(int fd_client, char* message_to_send);
#endif //GOPHERWIN_WINDOWS_SOCKET_H
