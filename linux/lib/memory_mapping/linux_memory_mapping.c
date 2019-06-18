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
#include "linux_memory_mapping.h"


int mappLinux(char* filename, int mod){

    struct stat mystat;
    void* pmap;

    int fd = open(filename, O_RDONLY);
    Assert( fd != -1 , "OpenError: ");

    if (fstat(fd, &mystat) < 0){
        perror("fstat");
        close(fd);
        exit(1);
    }


    //Execute mmap

    pmap = mmap(0, mystat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);


    if (pmap == MAP_FAILED){
        perror("mmap");
        close(fd);
        exit(1);
    }
    write(1, pmap, mystat.st_size); // Va cambiato con write su socket


    close(fd);

    return 0;

}
