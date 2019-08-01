#include <sys/socket.h>

#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <glob.h>
#include <fcntl.h>
#include <sys/stat.h>
// #include <tclDecls.h>
#include <errno.h>
#include <wait.h>


#include "definitions.h"
#include "protocol.h"
#include "socket.h"
#include "linux_socket.h"
#include "linux_files_interaction.h"
#include "files_interaction.h"

void close_mutex() {
    if (configs->mode_concurrency == M_THREAD) {
        if(pthread_mutex_destroy(&p_mutex) != 0){
            perror("pthread_mutex_destroy");
        }
    }
}

void start_mutex() {

    if (configs->mode_concurrency == M_THREAD) {
        printf("pthread_mutex_init/Lock setted \n");
        if(pthread_mutex_init(&p_mutex, NULL) != 0){
            perror("pthread_mutex_init");
        }
    }
}


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

//    printf("File descriptor for socket is %d\n", fd);
    int opts = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) != 0){
        perror("start_server/setsockopt");
        exit(-1);
    }


    // Bind the socket
    static struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // todo perche la dimensione e diversa?
    if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        return -1;
    printf("Socket bound to port %d\n", port);

    // Make it a listening socket
    if (listen(fd, queue_size) == -1) {
        perror("Listen crashed");
        return -1;
    }
    return fd;
}

int run_concurrency(struct ThreadArgs *args) {

    if (args->configs.mode_concurrency == M_THREAD) {
        pthread_t thread;
        /*pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);*/
        if ((pthread_create(&thread, NULL, handle_request_thread, (void *) args)) != 0) {
            // printf("%s\n", "Could not create thread, continue non-threaded...");
            perror("Could not create thread");
            // handle_request(req_fd);
        }
        //pthread_attr_destroy(&attr);
        //pthread_join(thread, NULL);
        return 0;
    } else if (args->configs.mode_concurrency == M_PROCESS) {

        pid_t pid_child = fork();
        if (pid_child < 0) {
            perror("run_concurrency/fork child failed");
        } else if (pid_child == 0) {
            // child
            handle_request(args);
            exit(0);
        } else {
            close(args->fd);
            free(args->ip_client);
            free(args);
        }
        return 0;
    } else {
        return -1;
    }
}


int linux_socket(struct Configs *configs) {
    fd_set rset;
    int n_ready;
    printf("%s\n", "Starting gopher server");
    int fd_server;
    if ((fd_server = start_server(configs->port_number, CONNECTION_QUEUE)) < 0) {
        perror(NULL);
        exit(-1);
    }


    // Accept connections, blocking
    int accept_fd;

//    printf("%s\n", "Going to acceptance");
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);


    FD_ZERO(&rset);
    int maxfdp1 = fd_server + 1;
    struct timeval timeout;
    timeout.tv_sec = SOCK_START_TIMEOUT;
    timeout.tv_usec = SOCK_START_TIMEOUT;

    for (int connection_counter = 0; MAX_CONNECTIONS_ALLOWED >= connection_counter; connection_counter++) {
//    for (;;) {
        FD_SET(fd_server, &rset);


        // if ((accept_fd = accept(fd_server, (struct sockaddr *) &client_addr, &slen)) == -1) { break; }

        if ((n_ready = select(maxfdp1, &rset, NULL, NULL, &timeout)) < 0) {
            if (errno == EINTR) continue;
            else {
                perror("select");
                return 1;
            }
        }
        if (0 == n_ready) {

            // Reset configs
            if (configs->reset_config != NULL) {
                //end_server(fd_server);
                printf("Reset configs\n");
                end_server(fd_server);
                return -1;
            }
            //printf("Reset socket continue %ls \n", configs->reset_config);
            timeout.tv_sec = SOCK_LOOP_TIMEOUT;
            timeout.tv_usec = SOCK_LOOP_TIMEOUT;

            continue;
        }
        if (FD_ISSET(fd_server, &rset)) {
            if ((accept_fd = accept(fd_server, (struct sockaddr *) &client_addr, &slen)) < 0) {
                if (errno == EINTR) continue;
                else {
                    perror("accept");
                    exit(1);
                }
            }


            char* clientname = calloc(INET_ADDRSTRLEN + 1, sizeof(char));
            if(clientname == NULL){
                perror("linux_socket/clientname  calloc");
                exit(-1);
            }
            inet_ntop( AF_INET, &client_addr.sin_addr, clientname, INET_ADDRSTRLEN);

            struct ThreadArgs *args = calloc(1, sizeof(struct ThreadArgs));
            args->configs = *configs;
            args->ip_client = clientname;
            args->fd = accept_fd;

            // QUI VA MULTICORE
            if (run_concurrency(args)) {
                perror("invalid option");
            }

            printf("%s\n", "Accepted request");

        }

    }

    // End server
    if (end_server(fd_server) < 0) {
        perror(NULL);
        return EXIT_FAILURE;
    }
    printf("%s\n", "Socket released");
    return EXIT_SUCCESS;
}


int get_line(char *buf, size_t size) {
    char *pos = memchr(buf, '\n', size);
    if (pos != NULL) {
        *(pos - 1) = 0;
        return 1;
    }
    return 0;
}
/*
void linux_sock_send_error(int *fd) {
    char error[] = "Unable to satisfy the request, retry later";
    send(*fd, error, sizeof(char) * strlen(error), 0);
}

void linux_sock_send_message(int *fd, char *error) {
    send(*fd, error, sizeof(char) * strlen(error), 0);
}*/


void *handle_request_thread(void *params) {
    pthread_detach(pthread_self());
    return handle_request(params);
}

void *handle_request(void *params) {
    char *buf;
    char *path;
    struct ThreadArgs *args;
    args = (struct ThreadArgs *) params;

    printf("%s\n", "------- new handle request ---------------");
//    printf("args: %d\n", args->fd);

    socket_read_request(args, &buf); // fill the buffer with the request

    socket_resolve_selector(args, buf, &path); // parse the request

    // todo fix resolve_selector come su linux
    printf("going to socket_manage_files - full required path: %s \n", path);
    socket_manage_files(path, buf, args); // send response
    printf("going to clean_request exiting handle_request\n");
    clean_request(path, buf, args);
    return 0;
    //return 0;

}





