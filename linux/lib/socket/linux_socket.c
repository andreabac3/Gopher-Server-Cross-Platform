//
// Created by valerioneri on 6/13/19.
//
#include <sys/socket.h>

#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

#include <glob.h>

#if defined(__unix__)

#include <avfs.h>
#include <protocol.h>

#endif


#include "linux_socket.h"
#include "linux_files_interaction.h"


#define CONNECTION_QUEUE 500

int end_server(int fd) {
    shutdown(fd, 2);
    return close(fd);
}

int start_server(unsigned int port, unsigned int queue_size) {
    int fd = 0;

    // signal(SIGPIPE, catch_sigpipe);

    // Create a socket
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) return fd;

    printf("File descriptor for socket is %d\n", fd);
    int opts = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));

    // Bind the socket
    // TODO This will prevent us to start up multiple servers
    static struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        return -1;
    printf("Socket bound to port %d\n", port);

    // Make it a listening socket
    if (listen(fd, queue_size) == -1) {
        perror("Listen crrashed");
        return -1;
    }
    return fd;
}

int linux_socket(struct Configs configs) {
    printf("%s\n", "Starting gophd...");
    int fd;
    if ((fd = start_server(configs.port_number, CONNECTION_QUEUE)) < 0) {
        perror(NULL);
        abort();
    }

    // Accept connections, blocking
    int accept_fd;
    //struct sockaddr *addr = NULL;
    socklen_t *length_ptr = NULL;

    //atexit(&main_shutdown);

    printf("%s\n", "Going to acceptance");
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);

    while ((accept_fd = accept(fd, (struct sockaddr *) &client_addr, &slen)) != -1) {
        int *req_fd = malloc(sizeof(int));
        *req_fd = accept_fd;
        printf("%u\n", client_addr.sin_addr.s_addr);
        char clientname[500];
        printf("Client Adress = %s\n", inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname)));
        printf("Client Adress = %s\n", clientname);


        /*
         * conntfd=accept(socketfd,(struct sockaddr*)&cli,&len);
    printf(“\nACCEPTED\n);

         */
        pthread_t thread;
        printf("%s\n", "Accepted request");
        if (pthread_create(&thread, NULL, handle_request, req_fd) != 0) {
            // printf("%s\n", "Could not create thread, continue non-threaded...");
            perror("Could not create thread, continue non-threaded...");
            // handle_request(req_fd);
        }
        //free(addr);
        free(length_ptr);
    }

    perror("Accept Failes");

    // End server
    if (end_server(fd) < 0) {
        perror(NULL);
        return EXIT_FAILURE;
    }
    printf("%s\n", "Socket released");
    return EXIT_SUCCESS;
}

#define BUFFER_SIZE 1024

int get_line(char *buf, size_t size) {
    char *pos = memchr(buf, '\n', size);
    if (pos != NULL) {
        *(pos - 1) = 0;
        return 1;
    }
    return 0;
}

void linux_sock_send_error(int *fd) {
    char error[] = "Unable to satisfy the request, retry later";
    send(*fd, error, sizeof(char) * strlen(error), 0);
}

void linux_sock_send_message(int *fd, char *error) {
    send(*fd, error, sizeof(char) * strlen(error), 0);
}


void *handle_request(void *args) {
    pthread_detach(pthread_self());


    printf("%s\n", "Running in thread");

    int *fd = (int *) args;


    int run = 1;
    int ptr = 0;
    ssize_t got_bytes = 0;
    char *buf = malloc(BUFFER_SIZE);
    while (run) {
        got_bytes = read(*fd, buf + ptr, BUFFER_SIZE - ptr);
        if (got_bytes <= 0) {
            buf[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if (get_line(buf, ptr)) {
            break;
        }
    }

    char *path = resolve_selector(NULL, buf);

    if (!file_exist(path)) {
        printf("SEND: Il file non esiste");

        // tell to client that file do not exists
        char *m = malloc(1);
        int err = protocol_response('3', buf, "/path/", "localhost", 7070, m);
        if (err != 0) {
            linux_sock_send_error(fd);
            // No need to free m, because calloc crashed.
        } else {
            send(*fd, m, sizeof(char) * strlen(m), 0);
            free(m);

            close(*fd);
            int ret = 1;
            pthread_exit(&ret);
        }
    }

    char code = getGopherCode(path);
    if (code < 0) {
        // error
        linux_sock_send_error(fd);
        close(*fd);
        int ret = 0;
        pthread_exit(&ret);
    }
    printf("Code: %d\n", code);
    if (code == '1') {
        // it's a directory
        printf("%s\n", "Directory");
        char *m = "Directory\n";
        send(*fd, m, sizeof(char) * strlen(m), 0);
        print_directory(path, &linux_sock_send_message, fd);
    } else {
        // it's some kind of files
    }


    printf("%s\n", "Responding");
    char *m = "bienvenue\n";
    send(*fd, m, sizeof(char) * strlen(m), 0);
    printf("%s\n", "Rresponded!");


    //shutdown(*fd, SHUT_WR);

    close(*fd);
    int ret = 0;
    pthread_exit(&ret);

    //return 0;

}



