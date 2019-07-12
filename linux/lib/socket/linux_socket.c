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
// #include <tclDecls.h>
#include <errno.h>


#include <socket.h>
#include "linux_socket.h"
#include "linux_files_interaction.h"

#define CONNECTION_QUEUE 500
#define MAX_CONNECTIONS_ALLOWED cnt++ < 1000

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

int run_concurrency(struct ThreadArgs *args) {

    if (args->configs.mode_concurrency == M_THREAD) {
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if ((pthread_create(&thread, &attr, handle_request_thread, (void *) args)) != 0) {
            // printf("%s\n", "Could not create thread, continue non-threaded...");
            perror("Could not create thread, continue non-threaded...");
            // handle_request(req_fd);
        }
        pthread_attr_destroy(&attr);
        return 0;
    } else if (args->configs.mode_concurrency == M_PROCESS) {
        if (fork() == 0) {
            // child
            handle_request(args);
            exit(0);
        } else {
            close(args->fd);
        }
        return 0;
    } else {
        fprintf(stderr, "linux_socket.c/run_concurrency");
        return -1;
    }
}


int linux_socket(struct Configs configs) {
    int cnt = 0;
    fd_set rset;
    int n_ready;
    printf("%s\n", "Starting gophd...");
    int fd;
    if ((fd = start_server(configs.port_number, CONNECTION_QUEUE)) < 0) {
        perror(NULL);
        abort();
    }

    // Accept connections, blocking
    int accept_fd;

    printf("%s\n", "Going to acceptance");
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);


    FD_ZERO(&rset);
    int maxfdp1 = fd + 1;


    while (MAX_CONNECTIONS_ALLOWED) {
        FD_SET(fd, &rset);

        // TODO if ((accept_fd = accept(fd, (struct sockaddr *) &client_addr, &slen)) == -1) { break; }

        if ((n_ready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) continue;
            else {
                perror("select");
                exit(1);
            }
        }
        if (FD_ISSET(fd, &rset)) { /* richiesta proveniente da client TCP */
            if ((accept_fd = accept(fd, (struct sockaddr *) &client_addr, &slen)) < 0) {
                if (errno == EINTR) continue;
                else {
                    perror("accept");
                    exit(1);
                }
            }


            //int *req_fd = malloc(sizeof(int));
            //*req_fd = accept_fd;
            printf("%u\n", client_addr.sin_addr.s_addr);
            char clientname[500];
            printf("Client Adress = %s\n", inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname)));
            printf("Client Adress = %s\n", clientname);

            struct ThreadArgs *args = calloc(1, sizeof(struct ThreadArgs));
            args->configs = configs;

            args->fd = accept_fd;

            // QUI VA MULTICORE
            run_concurrency(args);

            printf("%s\n", "Accepted request");

        }

    }

    perror("Accept Failes or while terminated");

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

void clean_request(char *path, char *buf, struct ThreadArgs *args) {

    if (path != NULL) {
        free(path);
    }
    free(buf);

    // shutdown(*fd, SHUT_WR);

    int err = close(args->fd);
    if (err != 0) {
        perror("Close in clean request");
    }
    int ret = 0;
    if (args->configs.mode_concurrency == M_THREAD) {
        free(args);
        pthread_exit(&ret);
    } else { // mode_concurrency == M_PROCESS
        free(args);
        exit(ret);
    }
}

void *handle_request_thread(void *params) {
    pthread_detach(pthread_self());
    handle_request(params);
}

void *handle_request(void *params) {

    printf("%s\n", "Running in thread");

    struct ThreadArgs *args;
    args = (struct ThreadArgs *) params;
    printf("port: %d\n", args->configs.port_number);

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
    char *path; // = calloc(1, sizeof(char));
    int retResolveSelector = resolve_selector(args->configs.root_dir, &path, buf);
    printf("%s", path);
    if (retResolveSelector == NO_FREE) {
        clean_request(NULL, buf, args);
    } else if (retResolveSelector == NEED_TO_FREE) {
        clean_request(path, buf, args);
    }
    if (!file_exist(path)) {
        printf("SEND: Il file non esiste");

        // tell to client that file do not exists
        char *m;
        int err = protocol_response('3', buf, "/path/", "localhost", 7070, &m);

        if (err != 0) {
            //linux_sock_send_error(args->fd);
            // No need to free m, because calloc crashed.
        } else {
            send(args->fd, m, sizeof(char) * strlen(m), 0);
            free(m);
            clean_request(path, buf, args);
        }
    }
    char code = getGopherCode(path);
    if (code < 0) {
        // error
        //linux_sock_send_error(args->fd);
        clean_request(path, buf, args);
    }
    printf("Code: %c\n", code);
    if (code == '1') {
        // it's a directory
        printf("%s\n", "Directory");
        print_directory(path, &linux_sock_send_message, &args->fd);
    } else {
        printf("%s\n", "filesssss");
        //printf("MIA FILE SIZE %jd\n", (intmax_t)sendFile(path));
        FILE *fp_FileToSend = sendFileToClient(path);
        int remain_data = fsize(fp_FileToSend);
        // int, int, off_t, off_t *, struct sf_hdtr *, int);
        //sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
        printf("%d", SendFile(args->fd, fp_FileToSend, remain_data));
        // it's some kind of files
    }

    clean_request(path, buf, args);
    fprintf(stderr, "%s Clean request morto\n", "sono messaggio di errore");

    //return 0;

}





