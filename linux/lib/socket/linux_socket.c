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
#include <libgen.h>
#include <ifaddrs.h>


#include "definitions.h"
#include "protocol.h"
#include "socket.h"
#include "linux_socket.h"
#include "linux_files_interaction.h"
#include "files_interaction.h"

void close_mutex() {
    if (configs->mode_concurrency == M_THREAD) {
        if (pthread_mutex_destroy(&p_mutex) != 0) {
            perror("pthread_mutex_destroy");
        }
    }
}

void start_mutex() {

    if (configs->mode_concurrency == M_THREAD) {
        printf("pthread_mutex_init/Lock setted \n");
        if (pthread_mutex_init(&p_mutex, NULL) != 0) {
            perror("pthread_mutex_init");
        }
    }
}


int end_server(int fd) {

    shutdown(fd, 2);
    return close(fd);
}

int start_server(unsigned int port, int queue_size) {
    if (queue_size < 1){
        return -1;
    }
    int fd = 0;

    // signal(SIGPIPE, catch_sigpipe);

    // Create a socket
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) return fd;

//    printf("File descriptor for socket is %d\n", fd);
    int opts = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) != 0) {
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
            return -1;
            // handle_request(req_fd);
        }
        //pthread_attr_destroy(&attr);
        //pthread_join(thread, NULL);
        return 0;
    } else if (args->configs.mode_concurrency == M_PROCESS) {

        pid_t pid_child = fork();
        if (pid_child < 0) {
            perror("run_concurrency/fork child failed");
            return -1;
        } else if (pid_child == 0) {
            // child
            handle_request(args);
            exit(0);
        } else { // parent
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


    for (;;) {
        FD_SET(fd_server, &rset);

        printf("sono qui\n");


        if ((n_ready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) {
                if (configs->reset_config != NULL) {
                    //end_server(fd_server);
                    printf("sono qua dentro\n");
                    printf("Reset configs\n");
                    end_server(fd_server);
                    return -1;
                }
                continue;
            } else {
                perror("select");
                continue;
            }
        }
        if (FD_ISSET(fd_server, &rset)) {
            if ((accept_fd = accept(fd_server, (struct sockaddr *) &client_addr, &slen)) < 0) {
                // TODO va levato il continue, se riceve un interrupt deve continuare ad eseguire con la richiesta
                if (errno == EINTR);// continue;
                else {
                    perror("accept");
                    // exit(1);
                    continue;
                }
            }


            char *clientname = calloc(INET_ADDRSTRLEN + 1, sizeof(char));
            if (clientname == NULL) {
                perror("linux_socket/clientname  calloc");
                // exit(-1);
                continue;
            }
            inet_ntop(AF_INET, &client_addr.sin_addr, clientname, INET_ADDRSTRLEN);

            struct ThreadArgs *args = calloc(1, sizeof(struct ThreadArgs));
            args->configs = *configs;
            args->ip_client = clientname;
            args->fd = accept_fd;

            // QUI VA MULTICORE
            if (run_concurrency(args) < 0) {
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

void *l_sendFile(void *args) {

    struct SendFileArgs *send_args = (struct SendFileArgs *) args;

    int fd_client = send_args->fd_client;
    char *message_to_send = send_args->message_to_send;
    int message_len = send_args->message_len;

    int bufferSize = 512;
    char buffer[BUFFER_SIZE];
    int sendPosition = 0;
    while (message_len > 0) {
        int chunkSize = message_len > bufferSize ? bufferSize : message_len;
        memcpy(buffer, message_to_send + sendPosition, chunkSize);
        chunkSize = send(fd_client, buffer, chunkSize, 0);
        if (chunkSize == -1) { break; }
        message_len -= chunkSize;
        sendPosition += chunkSize;
    }
    return NULL;
}

void *linux_sendFile(void *args) {

    struct SendFileArgs *send_args = (struct SendFileArgs *) args;

    int fd_client = send_args->fd_client;
    char *message_to_send = send_args->message_to_send;
    int message_len = send_args->message_len;


    int ret = send(fd_client, message_to_send, message_len, 0);

    if (0 > ret) {
        perror("socket.c/socket_send_message: send failed");
    }

    return NULL;
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

    printf("going to socket_manage_files - full required path: %s \n", path);
    socket_manage_files(path, buf, args); // send response
    printf("going to clean_request exiting handle_request\n");
    clean_request(path, buf, args);
    return 0;
    //return 0;

}

int blackListFile(char *baseDir, char *pathFile, char *black_listed_file) {
    char *base_name = basename(pathFile);
    if (strcmp(black_listed_file, base_name) != 0) {
        return 0;
    }
    char *base_dir_abs = realpath(baseDir, NULL);
    char *path_file_abs = realpath(pathFile, NULL);

    char *real_base_dir = dirname(base_dir_abs);
    char *path_file = dirname(path_file_abs);


    if (strcmp(real_base_dir, path_file) != 0) {
        return 0;

    }
    return 1;
}

int getServerIP(){
    struct ifaddrs *iflist, *iface;

    if (getifaddrs(&iflist) < 0) {
        perror("getifaddrs");
        return 1;
    }

    for (iface = iflist; iface; iface = iface->ifa_next) {
        int af = iface->ifa_addr->sa_family;
        const void *addr;
        char addrp[INET6_ADDRSTRLEN];

        switch (af) {
            case AF_INET:
                addr = &((struct sockaddr_in *)iface->ifa_addr)->sin_addr;
                break;
            case AF_INET6:
                addr = &((struct sockaddr_in6 *)iface->ifa_addr)->sin6_addr;
                break;
            default:
                addr = NULL;
        }

        if (addr) {
            if (inet_ntop(af, addr, addrp, sizeof addrp) == NULL) {
                perror("inet_ntop");
                continue;
            }
            if (strcmp(iface->ifa_name, "eno1") == 0 && addrp[3] == '.'){

                printf("Interface %s has address %s\n", iface->ifa_name, addrp);
                strncpy(ip_buffer, addrp, BUFFER_SIZE-1);
                ip_buffer[BUFFER_SIZE] = '\0';
                break;
            }

        }
    }

    freeifaddrs(iflist);
    return 0;
}
