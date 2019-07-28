//
// Created by valerioneri on 7/28/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <fcntl.h>
#include <definitions.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "utils.h"
#include "linux_pipe.h"

int socket_pipe_log_father(char *path, struct ThreadArgs *args, int dim_file_to_send, int *fd_pipe_log){
    close(fd_pipe_log[0]);
    struct PipeArgs pipeArgs1;
    pipeArgs1.path = path;
    pipeArgs1.ip_client = args->ip_client;
    pipeArgs1.dim_file = dim_file_to_send;
    write(fd_pipe_log[1], &pipeArgs1, sizeof(pipeArgs1));
    close(fd_pipe_log[1]);
    return 0;

}

int socket_pipe_log_child(int *fd_pipe_log){
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

    printf("FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file,
            data.ip_client);

    /*int err = */ dprintf(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file,
                           data.ip_client);
    //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
    perror("dprintf");
    //write(fd_log, "cia", sizeof("cia"));

    //printf("SONO N %d \n", n);
    close(fd_pipe_log[0]);

    printf("---- pid_log process close\n");
    return 0;
}


void socket_pipe_log(char* path, struct ThreadArgs *args ){
    pid_t pid_log;
    int fd_pipe_log[2];
    //FILE *fp_log = fopen(LOG_PATH, "a");
    if (pipe(fd_pipe_log) < 0) {
        perror("pipe");
    }

    pid_log = fork();
    int dim_file_to_send = 22;

    if (pid_log < 0) {
        perror("error in fork");
    } else if (pid_log > 0) {
        socket_pipe_log_father(path, args, dim_file_to_send, fd_pipe_log);
    } else if (pid_log == 0) {
        socket_pipe_log_child(fd_pipe_log);
        exit(0);
    }
}

void socket_mmap(){
    des_mutex_child = shm_open(MUTEX, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (des_mutex_child < 0) {
        perror("failure on shm_open on des_mutex");
        exit(1);

    }

    if (ftruncate(des_mutex_child, sizeof(pthread_mutex_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    mutex_child = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, des_mutex_child, 0);

    if (mutex_child == MAP_FAILED) {
        perror("Error on mmap on mutex\n");
        munmap(mutex_child, sizeof(pthread_mutex_t));
        exit(1);
    }

    des_cond_child = shm_open(OKTOWRITE, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (des_cond_child < 0) {
        perror("failure on shm_open on des_cond");
        exit(1);
    }

    if (ftruncate(des_cond_child, sizeof(pthread_cond_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    condition_child = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                              des_cond_child, 0);

    if (condition_child == MAP_FAILED) {
        perror("Error on mmap on condition\n");
        munmap(mutex_child, sizeof(pthread_mutex_t));
        munmap(condition_child, sizeof(pthread_cond_t));

        exit(1);
    }
}

int socket_pipe_process(){

    socket_mmap();


    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex_child, &mutexAttr);

    /* set condition shared between processes */
    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(condition_child, &condAttr);


    // main code
    if (pipe(fd_pipe) < 0) {
        perror("pipe");
        exit(-1);

    }
    child = fork();
    if (child < 0) {
        perror("failed child");
        exit(-1);
    } else if (child == 0) {
        printf("FIGLIO È PARTITO IN ATTESA SULLA COND VARIABLE\n");
        close(fd_pipe[1]);

        while (true) {
            printf("RISETTO TUTTE LE CONDIZIONI DA CAPO\n");

            pthread_mutex_lock(mutex_child);
            printf("pthread_mutex_lock\n");

            pthread_cond_wait(condition_child, mutex_child);
            printf("pthread_cond_wait \n");


            printf("Signaled by PARENT process, wake up!!!!!!!!\n");

            // TODO INSERT CONDITION VARIABLE and while true
            printf("---- child process wrote\n");
            //FILE* fp_fileLog = fopen(LOG_PATH, "w");

            int fd_log = open(LOG_PATH, O_WRONLY | O_APPEND);
            //FILE* fp_filelog= fdopen(fd_log, "a");
            printf("---- child process open\n");
            if (fd_log == -1) {
                //if (fp_fileLog == NULL){
                printf("sono bloccato");

                exit(-1);
            }
            //int n;
            fprintf(stderr, "%s\n", "sono debug3");
            printf("BOOOOOOL : %d", fd_is_valid(fd_pipe[0]));

            char message[BUFFER_SIZE * 2] = {0};
            ssize_t nread = read(fd_pipe[0], message, BUFFER_SIZE * 2);
            // ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
            // printf("%zu", nread);
            fprintf(stderr, "%s %zu\n", "read riuscito -> sono debug", nread);


            printf("---- child process read\n");

            dprintf(fd_log, "Byte %s", message);
            //dprintf(fd_log, "<%s>\n", "bho");

            //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
            perror("dprintf");
            //write(fd_log, "cia", sizeof("cia"));

            //printf("SONO N %d \n", n);
            pthread_mutex_unlock(mutex_child);

            printf("---- child process close\n");

        }
        pthread_condattr_destroy(&condAttr);
        pthread_mutexattr_destroy(&mutexAttr);
        pthread_mutex_destroy(mutex_child);
        pthread_cond_destroy(condition_child);
        shm_unlink(OKTOWRITE);
        shm_unlink(MESSAGE);
        shm_unlink(MUTEX);
        close(fd_pipe[0]);
        exit(0);
    }
    close(fd_pipe[0]);

    return 0;
}

int socket_pipe_log_server(char *path, struct ThreadArgs *args, int dim_file_to_send, int *fd_pipe_log){

    pthread_mutex_lock(mutex_child);

//    struct PipeArgs pipeArgs1;
//    pipeArgs1.path = path;
//    pipeArgs1.ip_client = args->ip_client;
//    pipeArgs1.dim_file = dim_file_to_send;
    char message[BUFFER_SIZE * 2 + 1] = {0};
    snprintf(message, BUFFER_SIZE * 2, "FileName: %s\t%d Byte \t IP Client: %s\n", path, dim_file_to_send,
           args->ip_client);

    int n_write = write(fd_pipe_log[1], message, BUFFER_SIZE*2);
    if (n_write == 0){
        perror("socket_pipe_log_server - nothing written on pipe");
    }
    if (n_write < 0){
        perror("socket_pipe_log_server - error on pipe");
    }
    printf("socket_pipe_log_server write on pipe %d\n", n_write);
    //close(fd_pipe_log[1]);

    //printf("SONO N %d \n", n);
    pthread_cond_signal(condition_child);
    pthread_mutex_unlock(mutex_child);

    return 0;
}
