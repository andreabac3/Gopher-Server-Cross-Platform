//
// Created by Valerio Neri on 2019-06-07.
//

#ifndef GOPHER_PROTOCOL_H
#define GOPHER_PROTOCOL_H

#endif //GOPHER_PROTOCOL_H

int protocol_response(char type, char *filename, char *path, const char *host, int port, char *result);
char * resolve_selector( char * filepath, const char * selector );
int print_directory(char* path, void (*socket_send_f)(int*, char*), int* fd);

