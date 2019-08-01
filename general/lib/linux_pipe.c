//
// Created by valerioneri on 7/28/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <fcntl.h>
#include <definitions.h>
#include "linux_pipe.h"

int socket_pipe_log_child(char* path, struct ThreadArgs *args, int dim_file_to_send, int fd_pipe_log[]){
    close(fd_pipe_log[0]);
    struct PipeArgs pipeArgs1;
    pipeArgs1.path = path;
    pipeArgs1.ip_client = args->ip_client;
    pipeArgs1.dim_file = dim_file_to_send;
    write(fd_pipe_log[1], &pipeArgs1, sizeof(pipeArgs1));
    close(fd_pipe_log[1]);

}

int socket_pipe_log_father(int fd_pipe_log[]){
    close(fd_pipe_log[1]);
    printf("---- pid_log process wrote\n");
    //FILE* fp_fileLog = fopen(LOG_PATH, "w");
    int fd_log = open(LOG_PATH, O_WRONLY | O_APPEND);
    //FILE* fp_filelog= fdopen(fd_log, "a");
    printf("---- pid_log process open\n");
    if (fd_log == -1) {
        //if (fp_fileLog == NULL){
        printf("sono bloccato");
        exit(-1);
    }
    //int n;
    struct PipeArgs data;

    //ssize_t nread = read(fd_pipe_log[0], &data, sizeof(data));
    ssize_t nread = read(fd_pipe_log[0], &data, sizeof(data));
    printf("%zu", nread);


    printf("%zu", nread);

    printf("---- pid_log process read\n");


    //printf("\n sono figlio :-> %s\n", data->ip_client);
    printf("FileName: %s\n", data.path);
    printf("%d Byte \n", data.dim_file);
    printf("IP Client: %s\n", data.ip_client);

    /*int err = */ dprintf(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file,
                           data.ip_client);
    //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
    perror("dprintf");
    //write(fd_log, "cia", sizeof("cia"));

    //printf("SONO N %d \n", n);
    close(fd_pipe_log[0]);

    printf("---- pid_log process close\n");
}


void socket_pipe_log(char* path, struct ThreadArgs *args, int dim_file_to_send){
    pid_t pid_log;
    int fd_pipe_log[2];
    //FILE *fp_log = fopen(LOG_PATH, "a");
    if (pipe(fd_pipe_log) < 0) {
        perror("pipe");
    }

    pid_log = fork();
//    int dim_file_to_send = 22;

    if (pid_log < 0) {
        perror("error in fork");
    } else if (pid_log > 0) {
        socket_pipe_log_child(path, args, dim_file_to_send, fd_pipe_log);
    } else if (pid_log == 0) {

        socket_pipe_log_father(fd_pipe_log);
        exit(0);
    }
}
