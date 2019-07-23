//
// Created by Andrea Bacciu on 2019-07-10.
//


#ifndef GOPHERLINUX_SOCKET_H
#define GOPHERLINUX_SOCKET_H

#include <stdio.h>
#include"definitions.h"


#define SEND_BUFFER_SIZE 512

int SendFile(int write_fd, FILE *read_fd);

FILE *sendFileToClient(int fd);

int fsize(FILE *fp);

void socket_read_request(struct ThreadArgs *args, char **buf);

void socket_resolve_selector(struct ThreadArgs *args, char *buf, char **path);

void clean_request(char *path, char *buf, struct ThreadArgs *args);

int socket_send_message(int fd, char *message_string);

void socket_manage_files(char *path, char *buf, struct ThreadArgs *args);

int windows_memory_mapping(int fd_client, char *path);

int w_sendFile(int fd_client, char *message_to_send);

#if defined(__unix__) || defined(__APPLE__)
//TODO Controllare se si puó lasciare qui
pthread_rwlock_t rwlock;
#endif


#endif //GOPHERLINUX_SOCKET_H
