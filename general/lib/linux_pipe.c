//
// Created by valerioneri on 7/28/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "definitions.h"
#include "utils.h"
#include "linux_pipe.h"

int socket_pipe_log_child(char *path, struct ThreadArgs *args, int dim_file_to_send, int fd_pipe_log[]) {
    close(fd_pipe_log[0]);
//    struct PipeArgs pipeArgs1;
//    pipeArgs1.path = path;
//    pipeArgs1.ip_client = args->ip_client;
//    pipeArgs1.dim_file = dim_file_to_send;
    char message[BUFFER_SIZE * 2];
    snprintf(message, BUFFER_SIZE * 2, "FileName: %s\t%d Byte \t IP Client: %s\n", path, dim_file_to_send,
             args->ip_client);
    write(fd_pipe_log[1], &message, sizeof(message));
    close(fd_pipe_log[1]);

    return 0;

}

int socket_pipe_log_father(int fd_pipe_log[]) {
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
    //struct PipeArgs data;
    char message[BUFFER_SIZE * 2];

    //ssize_t nread = read(fd_pipe_log[0], &data, sizeof(data));
    ssize_t nread = read(fd_pipe_log[0], &message, sizeof(message));
    printf("%zu", nread);


    printf("%zu", nread);

    printf("---- pid_log process read\n");


//    //printf("\n sono figlio :-> %s\n", data->ip_client);
//    printf("FileName: %s\n", data.path);
//    printf("%d Byte \n", data.dim_file);
//    printf("IP Client: %s\n", data.ip_client);

//    dprintf(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file, data.ip_client);
    // int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
    dprintf(fd_log, "%s", message);
    perror("dprintf");
    //write(fd_log, "cia", sizeof("cia"));

    //printf("SONO N %d \n", n);
    close(fd_pipe_log[0]);

    printf("---- pid_log process close\n");
    return 0;
}


void socket_pipe_log(char *path, struct ThreadArgs *args, int dim_file_to_send) {
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

/*
 * function to call form the main process
 * */

int socket_pipe_log_server(char *path, struct ThreadArgs *args, int dim_file_to_send, int *fd_pipe_log) {

    pthread_mutex_lock(mutex);

//    struct PipeArgs pipeArgs1;
//    pipeArgs1.path = path;
//    pipeArgs1.ip_client = args->ip_client;
//    pipeArgs1.dim_file = dim_file_to_send;
    char message[BUFFER_SIZE * 2 + 1] = {0};
    snprintf(message, BUFFER_SIZE * 2, "FileName: %s\t%d Byte \t IP Client: %s", path, dim_file_to_send,
             args->ip_client);

    int n_write = write(fd_pipe_log[1], message, BUFFER_SIZE * 2);
    if (n_write == 0) {
        perror("socket_pipe_log_server - nothing written on pipe");
    }
    if (n_write < 0) {
        perror("socket_pipe_log_server - error on pipe");
    }
    printf("socket_pipe_log_server write on pipe %d\n", n_write);
    //close(fd_pipe_log[1]);

    //printf("SONO N %d \n", n);
    if (pthread_cond_signal(condition) != 0) {
        perror("pthread_cond_signal faild");
    }
    pthread_mutex_unlock(mutex);
    fprintf(stderr, "socket_pipe_log_server/pthread_cond_signal and pthread_mutex_unlock made");
    return 0;
}

void socket_pipe_new_process() {
    des_mutex = shm_open(MUTEX, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (des_mutex < 0) {
        perror("failure on shm_open on des_mutex");
        exit(1);

    }

    if (ftruncate(des_mutex, sizeof(pthread_mutex_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, des_mutex, 0);

    if (mutex == MAP_FAILED) {
        perror("Error on mmap on mutex\n");
        munmap(mutex, sizeof(pthread_mutex_t));
        exit(1);
    }

    des_cond = shm_open(OKTOWRITE, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (des_cond < 0) {
        perror("failure on shm_open on des_cond");
        exit(1);
    }

    if (ftruncate(des_cond, sizeof(pthread_cond_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    condition = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                              des_cond, 0);

    if (condition == MAP_FAILED) {
        perror("Error on mmap on condition\n");
        munmap(mutex, sizeof(pthread_mutex_t));
        munmap(condition, sizeof(pthread_cond_t));

        exit(1);
    }
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutexAttr);

    // set condition shared between processes
    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(condition, &condAttr);


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

            if(pthread_mutex_lock(mutex) != 0){
                perror("pthread_mutex_lock failed");
            }
            if(printf("pthread_mutex_lock\n")!= 0){
                perror("pthread_mutex_lock failed");
            }

            pthread_cond_wait(condition, mutex);
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

            dprintf(fd_log, "Byte %s\n", message);
            //dprintf(fd_log, "<%s>\n", "bho");

            //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
            perror("dprintf");
            //write(fd_log, "cia", sizeof("cia"));

            //printf("SONO N %d \n", n);
            if(pthread_mutex_unlock(mutex) != 0){
                perror("pthread_mutex_unlock failed");
            }

            printf("---- child process close\n");

        }
        pthread_condattr_destroy(&condAttr);
        pthread_mutexattr_destroy(&mutexAttr);
        pthread_mutex_destroy(mutex);
        pthread_cond_destroy(condition);
        shm_unlink(OKTOWRITE);
        shm_unlink(MESSAGE);
        shm_unlink(MUTEX);
        close(fd_pipe[0]);
        exit(0);
    }
    close(fd_pipe[0]);

}

void pipe_child(){
    printf("RISETTO TUTTE LE CONDIZIONI DA CAPO\n");

    pthread_mutex_lock(mutex);
    fprintf(stderr, "pthread_mutex_lock\n");

    pthread_cond_wait(condition, mutex);
    fprintf(stderr, "pthread_cond_wait \n");


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
    pthread_mutex_unlock(mutex);
    fprintf(stderr, "%s\n", "Uscito dal mutex");
    //printf("BOOOOOOL : %d", fd_is_valid(fd_pipe[0]));

    char message[BUFFER_SIZE * 2] = {0};
    ssize_t nread = read(fd_pipe[0], message, BUFFER_SIZE * 2);
    // ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
    // printf("%zu", nread);
    fprintf(stderr, "-Read from pipe %zu %s \n", nread, message);


    printf("---- child process read\n");

    dprintf(fd_log, "Byte %s\n", message);
    //dprintf(fd_log, "<%s>\n", "bho");

    //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
    perror("dprintf");
    //write(fd_log, "cia", sizeof("cia"));

    //printf("SONO N %d \n", n);


    printf("---- child process close\n");
}

void socket_pipe_new_process2() {
    des_mutex = shm_open(MUTEX, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (des_mutex < 0) {
        perror("failure on shm_open on des_mutex");
        exit(1);
    }

    if (ftruncate(des_mutex, sizeof(pthread_mutex_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
                                     PROT_READ | PROT_WRITE, MAP_SHARED, des_mutex, 0);

    if (mutex == MAP_FAILED) {
        perror("Error on mmap on mutex\n");
        exit(1);
    }

    des_cond = shm_open(OKTOWRITE, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (des_cond < 0) {
        perror("failure on shm_open on des_cond");
        exit(1);
    }

    if (ftruncate(des_cond, sizeof(pthread_cond_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    condition = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t),
                                        PROT_READ | PROT_WRITE, MAP_SHARED, des_cond, 0);

    if (condition == MAP_FAILED) {
        perror("Error on mmap on condition\n");
        exit(1);
    }


    /* HERE WE GO */
/**************************************/

    /* set mutex shared between processes */
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutexAttr);

/* set condition shared between processes */
    pthread_condattr_t condAttr;
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(condition, &condAttr);

    /*************************************/

    __pid_t pid = fork();
    if (pid < 0) {
        perror("pipe_log fork failed");
    } else if (pid == 0) {

        printf("child who waits on condition\n");

        /*pthread_mutex_lock(mutex);
        pthread_cond_wait(condition, mutex);
        pthread_mutex_unlock(mutex);

        printf("Signaled by son process, wake up!!!!!!!!\n");*/
        while(true){
            pipe_child();
        }

        pthread_condattr_destroy(&condAttr);
        pthread_mutexattr_destroy(&mutexAttr);
        pthread_mutex_destroy(mutex);
        pthread_cond_destroy(condition);

        shm_unlink(OKTOWRITE);
        shm_unlink(MESSAGE);
        shm_unlink(MUTEX);
        exit(0);

    } else {

    }

    /*{

        sleep(10);

        pthread_mutex_lock(mutex);
        pthread_cond_signal(condition);
        printf("son signaled\n");
        pthread_mutex_unlock(mutex);
        exit(0);
    } */
}

/*
int socket_pipe_log_server(char *path, struct ThreadArgs *args, int dim_file_to_send, int *fd_pipe_log){

    pthread_mutex_lock(mutex);

//    struct PipeArgs pipeArgs1;
//    pipeArgs1.path = path;
//    pipeArgs1.ip_client = args->ip_client;
//    pipeArgs1.dim_file = dim_file_to_send;
    char message[BUFFER_SIZE * 2 + 1] = {0};
    snprintf(message, BUFFER_SIZE * 2, "FileName: %s\t%d Byte \t IP Client: %s\n", path, dim_file_to_send,
             args->ip_client);
    fprintf(stderr, "Writing %s\n", message);
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
    if(pthread_cond_signal(condition) != 0){
        perror("pthread_cond_signal faild");
    }
    pthread_mutex_unlock(mutex);
    fprintf(stderr, "signal and unlock\n");

    return 0;
}*/

void* create_shared_memory(size_t size) {
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_ANONYMOUS | MAP_SHARED;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return mmap(NULL, size, protection, visibility, -1, 0);
}

void socket_pipe_new_process_mmpa() {

    //void* sh_mutex = create_shared_memory(sizeof(pthread_mutex_t));

    //void* sh_cond = create_shared_memory(sizeof(pthread_mutex_t));

}
/*
void socket_pipe_new_process3() {

    pthread_cond_t* condition;
    pthread_mutex_t* mutex;
    char* message;
    int des_cond, des_msg, des_mutex;
    int mode = S_IRWXU | S_IRWXG;

    des_mutex = shm_open(MUTEX, O_CREAT | O_RDWR | O_TRUNC, mode);

    if (des_mutex < 0) {
        perror("failure on shm_open on des_mutex");
        exit(1);
    }

    if (ftruncate(des_mutex, sizeof(pthread_mutex_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    mutex = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t),
                                    PROT_READ | PROT_WRITE, MAP_SHARED, des_mutex, 0);

    if (mutex == MAP_FAILED ) {
        perror("Error on mmap on mutex\n");
        exit(1);
    }

    des_cond = shm_open(OKTOWRITE, O_CREAT | O_RDWR | O_TRUNC, mode);

    if (des_cond < 0) {
        perror("failure on shm_open on des_cond");
        exit(1);
    }

    if (ftruncate(des_cond, sizeof(pthread_cond_t)) == -1) {
        perror("Error on ftruncate to sizeof pthread_cond_t\n");
        exit(-1);
    }

    condition = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, des_cond, 0);

    if (condition == MAP_FAILED ) {
        perror("Error on mmap on condition\n");
        exit(1);
    }



    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutexAttr);

    pthread_condattr_t condAttr;
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(condition, &condAttr);


    if (!fork()) {

        sleep(1);

        pthread_mutex_lock(mutex);
        pthread_cond_signal(condition);
        printf("son signaled\n");
        pthread_mutex_unlock(mutex);
        exit(0);
    }

    else {

        printf("father waits on condition\n");

        pthread_mutex_lock(mutex);
        pthread_cond_wait(condition, mutex);
        pthread_mutex_unlock(mutex);

        printf("Signaled by son process, wake up!!!!!!!!\n");

        pthread_condattr_destroy(&condAttr);
        pthread_mutexattr_destroy(&mutexAttr);
        pthread_mutex_destroy(mutex);
        pthread_cond_destroy(condition);

        shm_unlink(OKTOWRITE);
        shm_unlink(MESSAGE);
        shm_unlink(MUTEX);

    }
}*/

