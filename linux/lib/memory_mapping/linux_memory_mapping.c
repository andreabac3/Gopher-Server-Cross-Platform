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
#include "definitions.h"
#include "linux_memory_mapping.h"


int linux_memory_mapping(int fd_client, char *filename) {
    struct stat sb;

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return -2;
    }

    if (fstat(fd, &sb) < 0) {
        perror("fstat");
        close(fd);
        return -1;
    }

    void* addr = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    FILE *fp_FileToSend = sendFileToClient(fd);
    int remain_data = fsize(fp_FileToSend);

    SendFile(fd_client, fp_FileToSend, remain_data);

    if (munmap(addr, sb.st_size) < 0) {
        int err = errno;
        perror("linux_socket.c/munmap failed: ");
    }
    fclose(fp_FileToSend);
    close(fd);
    return 0;
}

