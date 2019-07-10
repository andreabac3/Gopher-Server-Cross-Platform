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

#include "definitions.h"




#include <protocol.h>
#include <fcntl.h>
#include <sys/stat.h>


#include "linux_socket.h"
#include "linux_files_interaction.h"


#define CONNECTION_QUEUE 500
int SendFile(int write_fd, int read_fd, off_t filesize);
int end_server(int fd) {
    shutdown(fd, 2);
    return close(fd);
}


int sendFileToClient(char* pathFilename){
    int fd = open(pathFilename, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));

        exit(EXIT_FAILURE);
    }
    return fd;


}

off_t getFileSize(int fd){
    struct stat file_stat;

    /* Get file stats */
    if (fstat(fd, &file_stat) < 0)
    {
        fprintf(stderr, "Error fstat --> %s", strerror(errno));

        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "File Size: \n%d bytes\n", file_stat.st_size);
    return file_stat.st_size;
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
    //struct sockaddr *addr = NULL;*args->fd
    socklen_t *length_ptr = NULL;

    //atexit(&main_shutdown);

    printf("%s\n", "Going to acceptance");
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);

    struct ThreadArgs args;
    args.configs = configs;

    while ((accept_fd = accept(fd, (struct sockaddr *) &client_addr, &slen)) != -1) {
        int *req_fd = malloc(sizeof(int));
        *req_fd = accept_fd;
        printf("%u\n", client_addr.sin_addr.s_addr);
        char clientname[500];
        printf("Client Adress = %s\n", inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname)));
        printf("Client Adress = %s\n", clientname);


        args.fd = (int) accept_fd;

        pthread_t thread;
        printf("%s\n", "Accepted request");
        if (pthread_create(&thread, NULL, handle_request, (void *) &args) != 0) {
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


void *handle_request(void *params) {
    pthread_detach(pthread_self());


    printf("%s\n", "Running in thread");

    struct ThreadArgs *args;
    args = (struct ThreadArgs *) params;
    printf("port: %d\n",  args->configs.port_number);

    char *mm = "Directory\n";
    send(args->fd, mm, sizeof(char) * strlen(mm), 0);

    printf("%s\n", "Send ok");
    perror("Send");

    int run = 1;
    int ptr = 0;
    ssize_t got_bytes = 0;
    char *buf = malloc(BUFFER_SIZE);
    while (run) {
        got_bytes = read(args->fd, buf + ptr, BUFFER_SIZE - ptr);
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
            //linux_sock_send_error(args->fd);
            // No need to free m, because calloc crashed.
        } else {
            send(args->fd, m, sizeof(char) * strlen(m), 0);
            free(m);

            close(args->fd);
            int ret = 1;
            pthread_exit(&ret);
        }
    }

    char code = getGopherCode(path);
    if (code < 0) {
        // error
        //linux_sock_send_error(args->fd);
        close(args->fd);
        int ret = 0;
        pthread_exit(&ret);
    }
    printf("Code: %c\n", code);
    if (code == '1') {
        // it's a directory
        printf("%s\n", "Directory");
        char *m = "Directory\n";
        send(args->fd, m, sizeof(char) * strlen(m), 0);
        print_directory(path, &linux_sock_send_message, args->fd);
    } else {
        printf("%s\n", "filesssss");
        //printf("MIA FILE SIZE %jd\n", (intmax_t)sendFile(path));
        int fd_FileToSend = sendFileToClient(path);
        off_t remain_data = getFileSize(fd_FileToSend);
        // int, int, off_t, off_t *, struct sf_hdtr *, int);
        //sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
        SendFile(args->fd, fd_FileToSend, remain_data);
        // it's some kind of files
    }


    printf("%s\n", "Responding");
    char *m = "bienvenue\n";
    send(args->fd, m, sizeof(char) * strlen(m), 0);
    printf("%s\n", "Rresponded!");


    //shutdown(*fd, SHUT_WR);

    close(args->fd);
    int ret = 0;
    pthread_exit(&ret);

    //return 0;

}

int SendFile(int write_fd, int read_fd, off_t filesize){
    size_t dim = 100;
    ssize_t n = 0;
    char buffer[dim];
    while(filesize > 0){

        if ((n = fread(read_fd, buffer, dim)) == -1){
            return -1;
        }
        if (send(write_fd, buffer, (size_t) n, 0) == -1){
            return -2;
        }
        filesize -= dim;
    }
    return 0;
}



