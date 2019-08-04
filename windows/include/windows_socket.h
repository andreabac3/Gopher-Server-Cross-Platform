#ifndef GOPHERWIN_WINDOWS_SOCKET_H
#define GOPHERWIN_WINDOWS_SOCKET_H

//int windows_socket_runner();

#include "definitions.h"
int windows_socket_runner(struct Configs *configs);
DWORD WINAPI w_sendFile(PVOID args);
DWORD WINAPI handle_request(void *params);
int blackListFile(char *baseDir, char *pathFile, char *black_listed_file);
#endif //GOPHERWIN_WINDOWS_SOCKET_H

