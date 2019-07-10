#include <memory.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "socket.h"
#include "errno.h"
#include "socket.h"

int SendFile(int write_fd, FILE* read_fd, int filesize){
    ssize_t n = 0;
    char buffer[SEND_BUFFER_SIZE + 1] = { 0 } ;
    while(filesize > 0){
        if (filesize < SEND_BUFFER_SIZE){
            //fill buff with zeros
            memset(buffer, 0, sizeof(buffer));
        }

        if ((n = fread(buffer, sizeof(char), SEND_BUFFER_SIZE, read_fd)) == -1){
            return -1;
        }
        printf("send_buffer: |%s| %zu %zu\n", buffer, n, strlen(buffer));
        if (send(write_fd, buffer, (size_t) n, 0) == -1){
            return -2;
        }
        filesize -= n;
    }
    return 0;
}



FILE* sendFileToClient(char* pathFilename){
    FILE* fp = fopen(pathFilename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));

        exit(EXIT_FAILURE);
    }
    return fp;


}

int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}
