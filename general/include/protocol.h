#ifndef GOPHER_PROTOCOL_H
#define GOPHER_PROTOCOL_H

#endif //GOPHER_PROTOCOL_H

int protocol_response(char type, char *filename, char *path, const char *host, int port, char **result);
int resolve_selector(char* gopher_root, char** filepath, const char* selector);
int print_directory(char *path, int (*socket_send_f)(int, char *), int fd, int port, char *buf);
char win_getGopherCode(char* path);


