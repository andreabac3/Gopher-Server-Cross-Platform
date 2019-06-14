//
// Created by valerioneri on 6/13/19.
//
#include <sys/socket.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "linux_socket.h"

int linux_socket(struct Configs configs)
{
    printf("%s\n", "Starting gophd...");
    int fd;
    if( (fd = start_server( LISTEN_PORT, CONNECTION_QUEUE )) < 0 ){
        perror(NULL);
        abort();
    }

    // Accept connections, blocking
    int accept_fd;
    struct sockaddr *addr = NULL;
    socklen_t *length_ptr = NULL;

    //atexit(&main_shutdown);

    while( (accept_fd = accept(fd, addr, length_ptr)) != -1 ) {
        int * req_fd = malloc(sizeof(int));
        *req_fd = accept_fd;
        pthread_t thread;
        if( pthread_create(&thread, NULL, handle_request, req_fd) != 0 ){
            printf("%s\n", "Could not create thread, continue non-threaded...");
            handle_request(req_fd);
        }
        free(addr);
        free(length_ptr);
    }

    // End server
    if( end_server(fd) < 0 ) {
        perror(NULL);
        return EXIT_FAILURE;
    };
    printf("%s\n", "Socket released");
    return EXIT_SUCCESS;
}

void * handle_request(void * args){

}

int end_server(int fd){
    shutdown(fd, 2);
    return close( fd );
}

int start_server(unsigned int port, unsigned int queue_size ){
    int fd = 0;

    // signal(SIGPIPE, catch_sigpipe);

    // Create a socket
    fd = socket( PF_INET, SOCK_STREAM, 0 );
    if( fd == -1 ) return fd;

    printf("File descriptor for socket is %d\n", fd);
    int opts = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));

    // Bind the socket
    // TODO This will prevent us to start up multiple servers
    /*struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons( port );
    serv_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        return -1;
    printf("Socket bound to port %d\n", port);

    // Make it a listening socket
    if( listen(fd, queue_size) == -1 )
        return -1;*/
    return fd;
}


