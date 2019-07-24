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


#if __linux__

#include <sys/mman.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22)
#define _MAP_POPULATE_AVAILABLE
#endif
#endif

#ifdef _MAP_POPULATE_AVAILABLE
#define MMAP_FLAGS (MAP_PRIVATE | MAP_POPULATE)
#else
#define MMAP_FLAGS MAP_PRIVATE
#endif

int linux_memory_mapping(int fd_client, char *filename, int mode_concurrency) {
    struct stat sb;

    int fd = open(filename, O_RDWR);

    if (mode_concurrency == M_PROCESS){
        lockf(fd, F_LOCK, 0);
    }else{
        pthread_rwlock_rdlock(&rwlock);
    }

    if (fd < 0) {
        return -2;
    }

    if (fstat(fd, &sb) < 0) {
        perror("fstat");
        lockf(fd, F_ULOCK, 0);
        close(fd);
        return -1;
    }

    /*
     * MAP_PRIVATE = Create a private copy-on-write mapping.  Updates to the mapping are not visible to other processes
     *               mapping the same file.
     * PROT_READ = Pages may be read.

     */


    void *addr = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        lockf(fd, F_ULOCK, 0);
        close(fd);

        return -1;
    }

    FILE *fp_FileToSend = sendFileToClient(fd);
    int remain_data = fsize(fp_FileToSend);

    SendFile(fd_client, fp_FileToSend);

    if (munmap(addr, sb.st_size) < 0) {
        // int err = errno;
        perror("linux_socket.c/munmap failed: ");
    }
    fclose(fp_FileToSend);

    if (mode_concurrency == M_PROCESS){
        lockf(fd, F_ULOCK, 0);
    }else{
        pthread_rwlock_rdlock(&rwlock);
    }


    close(fd);
    return remain_data;
}

