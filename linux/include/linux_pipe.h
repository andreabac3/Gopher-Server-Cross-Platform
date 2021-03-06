#ifndef GOPHERLINUX_LINUX_PIPE_H
#define GOPHERLINUX_LINUX_PIPE_H

void socket_pipe_log(char* path, struct ThreadArgs *args, int dim_file_to_send);
int socket_pipe_log_server(char *path, struct ThreadArgs *args, int dim_file_to_send, int fd_pipe_log_write) ;
void socket_pipe_multiple_process(int *fd_pipe);
void socket_pipe_single_process(int *fd_pipe);
int socket_pipe_log_server_single_process(char *path, struct ThreadArgs *args, int dim_file_to_send, int fd_pipe_log_write);
void socket_pipe_new_process2();
void socket_pipe_new_process3();

#endif //GOPHERLINUX_LINUX_PIPE_H
