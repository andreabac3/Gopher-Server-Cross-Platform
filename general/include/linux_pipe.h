//
// Created by valerioneri on 7/28/19.
//

#ifndef GOPHERLINUX_LINUX_PIPE_H
#define GOPHERLINUX_LINUX_PIPE_H

void socket_pipe_log(char* path, struct ThreadArgs *args );
int socket_pipe_process();
int socket_pipe_log_server(char *path, struct ThreadArgs *args, int dim_file_to_send, int *fd_pipe_log);

#endif //GOPHERLINUX_LINUX_PIPE_H
