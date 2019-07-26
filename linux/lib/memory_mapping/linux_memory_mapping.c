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
#include "linux_memory_mapping.h"
#include <string.h>

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

void* linux_memory_mapping(void *params) {
    struct MemoryMappingArgs* args = (struct MemoryMappingArgs *) params;
    printf("SONO FDDDDDDDDDDDDDDDDDDD %s\n", args->path);
    printf("FD %d \n PATH %s \n %d\n", args->fd, args->path, args->mode_concurrency) ;
    int ret = -1;
    struct stat sb;

    int fd = open(args->path, O_RDONLY);

    if (args->mode_concurrency == M_PROCESS){
        lockf(fd, F_LOCK, 0);
    }else{
        pthread_mutex_lock(&p_mutex);
    }

    if (fd < 0) {

        //pthread_exit(&ret);
    }


    if (fstat(fd, &sb) < 0) {
        perror("fstat");
        lockf(fd, F_ULOCK, 0);
        close(fd);
        //pthread_exit(&ret);
    }

    /*
     * MAP_PRIVATE = Create a private copy-on-write mapping.  Updates to the mapping are not visible to other processes
     *               mapping the same file.
     * PROT_READ = Pages may be read.

     */


    void *addr = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    printf("%s", (char*)addr);

    if (addr == MAP_FAILED) {
        perror("mmap");
        lockf(fd, F_ULOCK, 0);
        printf("SONO PRIMA DI SENDDDDDDDDDDDDD FILEEEEEEEEEEEEEEEEEEEEEEEEEEE233333333333333@@@@@@@@@@2");

        close(fd);
    }


    //SendFileMapped(args->fd, (char*) addr, fileSize(fd));
    l_sendFile(args->fd, (char*)addr, fileSize(fd));
    if (munmap(addr, sb.st_size) < 0) {
        // int err = errno;
        perror("linux_socket.c/munmap failed: ");
    }

    if (args->mode_concurrency == M_PROCESS){
        lockf(fd, F_ULOCK, 0);
    }else{
        pthread_mutex_unlock(&p_mutex);
    }


    close(fd);
    ret = 0;
    close(args->fd);
    pthread_exit(&ret);
}

int fileSize(int fd) {
    struct stat s;
    if (fstat(fd, &s) == -1) {
        int saveErrno = errno;
        fprintf(stderr, "fstat(%d) returned errno=%d.", fd, saveErrno);
        return(-1);
    }
    return(s.st_size);
}

int l_sendFile(int fd_client, char *message_to_send, int message_len) {
    int bufferSize = 512;
    char buffer[BUFFER_SIZE];
    int sendPosition = 0;
    while (message_len > 0) {
        int chunkSize = message_len > bufferSize ? bufferSize : message_len;
        memcpy(buffer, message_to_send + sendPosition, chunkSize);
        chunkSize = send(fd_client, buffer, chunkSize, 0);
        // TODO controllare send
        if (chunkSize == -1) { break; }
        message_len -= chunkSize;
        sendPosition += chunkSize;
    }
    return 0;
}

