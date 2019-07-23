#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#ifdef _WIN32

#include <winsock.h>
#include <windows_protocol.h>
#include <io.h>
#include <windows_memory_mapping.h>

#endif

#if defined(__unix__) || defined(__APPLE__)

#include <sys/socket.h>
#include <zconf.h>
#include <pthread.h>
#include <wait.h>
#include <fcntl.h>
#include "linux_memory_mapping.h"
#include "linux_files_interaction.h"

#endif

#include "protocol.h"
#include "socket.h"
#include "utils.h"
#include "files_interaction.h"


int write_to_log(struct PipeArgs *data);


int SendFile(int write_fd, FILE *read_fd) {
    ssize_t n = 0;
    int filesize = fsize(read_fd);
    char buffer[SEND_BUFFER_SIZE + 1] = {0};
    while (filesize > 0) {
        if (filesize < SEND_BUFFER_SIZE) {
            //fill buff with zeros
            memset(buffer, 0, sizeof(buffer));
        }

        if ((n = fread(buffer, sizeof(char), SEND_BUFFER_SIZE, read_fd)) == -1) {
            return -1;
        }
        // printf("send_buffer: |%s| %zu %zu\n", buffer, n, strlen(buffer));
        if (send(write_fd, buffer, (size_t) n, 0) == -1) {
            return -2;
        }
        filesize -= n;
    }
    return 0;
}


FILE *sendFileToClient(int fd) {
    //FILE *fp = fopen(pathFilename, "rb");
    FILE *fp = fdopen(fd, "rb");
    printf("sono qui");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fp;
}

int fsize(FILE *fp) {
    int prev = ftell(fp);
    if (prev == -1L) {
        fclose(fp);
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    if (sz == -1L) {
        fclose(fp);
        return -1;
    }
    fseek(fp, prev, SEEK_SET); //go back to where we were
    return sz;
}

/*
 * frees the memory needed by the request do nothing if NULL is passed
 * and close the socket file descriptor always
 */
#ifdef _WIN32

void clean_request(char *path, char *buf, struct ThreadArgs *args) {

    if (path != NULL) {
        free(path);
    }
    if (buf != NULL) {
        free(buf);
    }

    // shutdown(*fd, SHUT_WR);

    int err = closesocket(args->fd);
    if (err != 0) {
        perror("Close in clean request");
    }
    int ret = 0;
    if (args->configs.mode_concurrency == M_THREAD) {
        free(args);
        ExitThread(ret);
    } else { // mode_concurrency == M_PROCESS
        free(args);
        ExitThread(ret);
    }
}

#endif
#if defined(__unix__) || defined(__APPLE__)

void clean_request(char *path, char *buf, struct ThreadArgs *args) {

    if (path != NULL) {
        free(path);
    }
    if (buf != NULL) {
        free(buf);
    }

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
        pthread_exit(&ret);
    }
}

#endif

/*
 * Wrapper for protocol.resolve_selector, control errors and clean the request
 * */
void socket_resolve_selector(struct ThreadArgs *args, char *buf, char **path) {
    printf("Selector string: %s\n", buf);
    //char *path;
    int ret_resolve_relector = resolve_selector(args->configs.root_dir, path, buf);
    if (ret_resolve_relector == NO_FREE) {
        clean_request(NULL, buf, args);
    } else if (ret_resolve_relector == NEED_TO_FREE) {
        clean_request(*path, buf, args);
    }
}

/*
 * socket_read_request read the request from the client and fill the buffer with the content.
 * */
void socket_read_request(struct ThreadArgs *args, char **buf) {

    *buf = calloc(BUFFER_SIZE, sizeof(char));
    if (*buf == NULL) {
        clean_request(NULL, NULL, args);
    }

    int ptr = 0;
    ssize_t got_bytes = 0;

    while (true) {
        got_bytes = recv(args->fd, *buf + ptr, BUFFER_SIZE - ptr, 0);
        if (0 > got_bytes) {
            clean_request(NULL, *buf, args);
        }
        if (0 == got_bytes) {
            (*buf)[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if (ut_get_line(*buf, ptr)) {
            break;
        }
    }
}

int socket_send_message(int fd, char *message_string) {

    printf("socket_send_message: %d, %s", fd, message_string);
    int ret = send(fd, message_string, sizeof(char) * strlen(message_string), 0);
    if (0 > ret) {
        perror("socket.c/socket_send_message: send failed");
        return ret;
    }
    return 0;
}

void socket_manage_files(char *path, char *buf, struct ThreadArgs *args) {
    int type_file = file_type(path);
    if (FILES_NOT_EXIST == type_file) {
        printf("SEND: Il file non esiste");

        // tell to client that file do not exists
        char *m;
        int err = protocol_response('3', buf, path, "localhost", args->configs.port_number, &m);
        if (err != 0) {
            clean_request(path, buf, args);
        } else {
            printf("%s", m);
            //send(args->fd, m, sizeof(char) * strlen(m), 0);
            socket_send_message(args->fd, m);
            free(m);
        }
        clean_request(path, buf, args);
    }

    // Todo check if file or directory
    char code = getGopherCode(path);
    if (code < 0) {
        clean_request(path, buf, args);
    }
    printf("Code: %d\n", code);

    if (FILES_IS_DIRECTORY == type_file) {
        // it's a directory
        printf("%s\n", "iDirectory");
        char *m = "iDirectory\n";
        send(args->fd, m, sizeof(char) * strlen(m), 0);
        print_directory(path, &socket_send_message, args->fd, args->configs.port_number);
    } else if (FILES_IS_REG_FILE == type_file) { // FILES_IS_FILE
        // it's some kind of files
        printf("%s\n", "filesssss");

        /*
         * TODO Usa la CreateFile per aprire il file mettendo dwShareMode a 0,
         * questo crea accesso esclusivo sia per Thread che per Processi
        */




#ifdef _WIN32
        windows_memory_mapping(args->fd, path);
        //printf("%d", SendFile(args->fd, fp_FileToSend));
#endif
#if defined(__unix__) || defined(__APPLE__)
        int dim_file_to_send = linux_memory_mapping(args->fd, path, args->configs.mode_concurrency);
        printf("%s\n", "fino qua ci sono à##################");
        pid_t child;
        int fd_pipe[2];
        //FILE *fp_log = fopen(LOG_PATH, "a");
        if (pipe(fd_pipe) < 0) {
            perror("pipe");
        }
        struct PipeArgs *pipeArgs1;
        int st;
        pipeArgs1->path = path;
        pipeArgs1->ip_client = args->ip_client;
        child = fork();

        if (child > 0) {
            write(fd_pipe[1], &pipeArgs1, sizeof(struct PipeArgs));
            wait(&st);
        } else if (child == 0) {
            printf("---- child process wrote\n");

            struct PipeArgs *data;
            close(fd_pipe[1]);
            while (read(fd_pipe[0], &data, sizeof(data)) > 0) {
                printf("\n sono figlio :-> %s\n", data->ip_client);

            }

            //printf("SONO N %d \n", n);
            close(fd_pipe[0]);
            exit(0);
        }
        fprintf(stdout, "parent process wrote it after fork!\n");


#endif
        //fclose(fp_FileToSend);
        clean_request(path, buf, args);
        //return 0;
    }
}

int write_to_log(struct PipeArgs *data) {
    FILE *fp_log = fopen("../gopher_log_file.txt", "a");
    if (fp_log == NULL){
        printf("%s","PERCHÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉ");
        return -1;
    }
    fprintf(fp_log, "%s", "ciaoo");
    printf("sono dentro la funzionee");
    fclose(fp_log);

}
