//
// Created by Andrea Bacciu on 2019-07-10.
//

#include <stdio.h>


#ifndef GOPHERLINUX_SOCKET_H
#define GOPHERLINUX_SOCKET_H
#define SEND_BUFFER_SIZE 4
int SendFile(int write_fd, FILE* read_fd, int filesize);
FILE* sendFileToClient(char* pathFilename);
int fsize(FILE *fp);
#endif //GOPHERLINUX_SOCKET_H
