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
#include "definitions.h"
#include "linux_memory_mapping.h"


int mappLinux(char* filename, struct mapFileStruct* mfile_struct){


    //void* addr;
    //void* addr = mfile_struct->addr;
    int fd = open(filename, O_RDONLY);

    Assert( fd != -1 , "OpenError: ");
    mfile_struct->fd = fd;


    if (fstat(fd, &mfile_struct->sb) < 0){
        perror("fstat");
        close(fd);
        exit(1);
    }

    //Execute mmap

    mfile_struct->addr = mmap(0, mfile_struct->sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);


    if (mfile_struct->addr == MAP_FAILED){
        perror("mmap");
        close(fd);
        exit(-1);
    }

    //write(1, pmap, mystat.st_size); // Va cambiato con write su socket

    return 0;

}
