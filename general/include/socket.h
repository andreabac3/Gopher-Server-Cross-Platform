#pragma once

#ifndef GOPHERLINUX_SOCKET_H
#define GOPHERLINUX_SOCKET_H

#include <stdio.h>
#include"definitions.h"


int socket_send_error_to_client(char *path, char *buf, struct ThreadArgs *args);

int socket_drain_tcp(int fd_client);

int SendFile(int write_fd, FILE *read_fd);

FILE *sendFileToClient(int fd);

int fsize(FILE *fp);

int socket_read_request(struct ThreadArgs *args, char **buf);

void socket_resolve_selector(struct ThreadArgs *args, char *buf, char **path);

int vecchiafork(char *path, char *ip_client, int dim_file_to_send);

void clean_request(char *path, char *buf, struct ThreadArgs *args);

int socket_send_message(int fd, char *message_string);

void socket_manage_files(char *path, char *buf, struct ThreadArgs *args);

int socket_send_too_long_error_to_client(char *path, char *buf, struct ThreadArgs *args);

#if defined(__unix__) || defined(__APPLE__)

int writeToPipe(char *path, char *ip_client);

pthread_mutex_t p_mutex;
#endif


#ifdef _WIN32
HANDLE pipe_read;
HANDLE pipe_write;
#endif


#endif //GOPHERLINUX_SOCKET_H
