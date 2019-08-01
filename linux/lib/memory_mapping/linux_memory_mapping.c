//
// Created by Andrea Bacciu on 2019-06-07.
//

#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/mman.h>
#include <socket.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "linux_memory_mapping.h"

#if __linux__

#include <sys/mman.h>
#include <linux/version.h>
#include <sys/socket.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22)
#define _MAP_POPULATE_AVAILABLE
#endif
#endif

#ifdef _MAP_POPULATE_AVAILABLE
#define MMAP_FLAGS (MAP_PRIVATE | MAP_POPULATE)
#else
#define MMAP_FLAGS MAP_PRIVATE
#endif

int linux_memory_mapping(void *params) {

    struct MemoryMappingArgs *args = (struct MemoryMappingArgs *) params;

//    printf("FD %d \n PATH %s \n %d\n", args->fd, args->path, args->mode_concurrency);
    struct stat sb;

    int fd = open(args->path, O_RDONLY);
    if (fd == -1) {
        perror("linux_memory_mapping/open");
        return -1;
    }

    if (args->mode_concurrency == M_PROCESS) {
        if(lockf(fd, F_LOCK, 0) != 0){
            perror("linux_memory_mapping/lockf failed");
        }
    } else {
        if(pthread_mutex_lock(&p_mutex)!= 0){
            perror("linux_memory_mapping/pthread_mutex_lock failed");
        }
    }

    if (fstat(fd, &sb) < 0) {
        perror("linux_memory_mapping/fstat");
        if(lockf(fd, F_ULOCK, 0)!= 0){
            perror("linux_memory_mapping/lockf failed");
        }
        close(fd);
        return -1;
        //pthread_exit(&ret);
    }

    /*
     * MAP_PRIVATE = Create a private copy-on-write mapping.  Updates to the mapping are not visible to other processes
     *               mapping the same file.
     * PROT_READ = Pages may be read.

     */

    if(sb.st_size == 0){
        return 0;
    }

    void *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (addr == MAP_FAILED) {
        perror("linux_memory_mapping/mmap");
        if(lockf(fd, F_ULOCK, 0)!= 0){
            perror("linux_memory_mapping/lockf failed");
        }

        close(fd);
        return -1;
    }


    //SendFileMapped(args->fd, (char*) addr, fileSize(fd));

    struct SendFileArgs send_args;
    send_args.fd_client = args->fd;
    send_args.message_to_send = (char *) addr;
    send_args.message_len = fileSize(fd);

    pthread_t thread;
    if ((pthread_create(&thread, NULL, l_sendFile, (void *) &send_args)) != 0) {
            perror("Could not create thread, continue non-threaded...");
            l_sendFile((void *) &send_args);
        }
    pthread_join(thread, NULL);
    // l_sendFile(&send_args);

    if (munmap(addr, sb.st_size) < 0) {
        perror("linux_socket.c/munmap failed: ");
    }

    if (args->mode_concurrency == M_PROCESS) {
        if(lockf(fd, F_ULOCK, 0)!= 0){
            perror("linux_memory_mapping/lockf failed");
        }
    } else {
        if(pthread_mutex_unlock(&p_mutex)!= 0){
            perror("linux_memory_mapping/pthread_mutex_unlock failed");
        }
    }


    close(fd);
    close(args->fd);
    return sb.st_size;
}

int fileSize(int fd) {
    struct stat s;
    if (fstat(fd, &s) == -1) {
        int saveErrno = errno;
        fprintf(stderr, "fstat(%d) returned errno=%d.", fd, saveErrno);
        return (-1);
    }
    return (s.st_size);
}

void * l_sendFile(void *args) {

    struct SendFileArgs *send_args = (struct SendFileArgs* ) args;

    int fd_client = send_args->fd_client;
    char *message_to_send = send_args->message_to_send;
    int message_len = send_args->message_len;

    int bufferSize = 512;
    char buffer[BUFFER_SIZE];
    int sendPosition = 0;
    while (message_len > 0) {
        int chunkSize = message_len > bufferSize ? bufferSize : message_len;
        memcpy(buffer, message_to_send + sendPosition, chunkSize);
        chunkSize = send(fd_client, buffer, chunkSize, 0);
        if (chunkSize == -1) { break; }
        message_len -= chunkSize;
        sendPosition += chunkSize;
    }
    return NULL;
}

