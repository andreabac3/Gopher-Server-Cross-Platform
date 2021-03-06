#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#ifdef _WIN32

#include <winsock.h>
#include <windows_protocol.h>
#include <io.h>
#include <windows_memory_mapping.h>
#include <process.h>
#include <windows_pipe.h>
#include <windows_utils.h>


#endif

#if defined(__unix__) || defined(__APPLE__)

#include <sys/socket.h>
#include <zconf.h>
#include <pthread.h>

#include <fcntl.h>
#include <libgen.h>
#include "linux_memory_mapping.h"
#include "linux_files_interaction.h"
#include "linux_pipe.h"


#endif

#include "protocol.h"
#include "socket.h"
#include "utils.h"
#include "files_interaction.h"
#include "definitions.h"


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
        // log_ut("send_buffer: |%s| %zu %zu\n", buffer, n, strlen(buffer));
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
    log_ut("sono qui");
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


    //int ret = 0;
    if (args->configs.mode_concurrency == M_THREAD) {
        int err = closesocket(args->fd);
        if (err != 0) {
            perror("Close in clean request");
        }
        free(args);
        //ExitThread(ret);
        _endthread();
    } else { // mode_concurrency == M_PROCESS
#if defined(__unix__) || defined(__APPLE__)
        int err = closesocket(args->fd);
        if (err != 0) {
            perror("Close in clean request");
        }
#endif
        free(args);
        perror("i processi ancora non sono stati implementati");
        exit(0);
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
    if (args->type_Request) {
        //int err = close(args->fd);
        int err = shutdown(args->fd, SHUT_RDWR );
        err += close(args->fd);
        if (err != 0) {
            perror("Close in clean request");
        }
    }
    int ret = 0;
    free(args->ip_client);
    if (args->configs.mode_concurrency == M_THREAD) {
        free(args);
        pthread_exit(&ret);
    } else { // mode_concurrency == M_PROCESS
        free(args);
        exit(ret);
    }
}

#endif

/*
 * Wrapper for protocol.resolve_selector, control errors and clean the request
 * */
void socket_resolve_selector(struct ThreadArgs *args, char *buf, char **path) {
    //log_ut("Selector string: %s\n", buf);
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


int socket_read_request(struct ThreadArgs *args, char **buf) {

    int ptr = 0;
    ssize_t got_bytes = 0;

    *buf = calloc(BUFFER_SIZE + 1, sizeof(char));
    if (*buf == NULL) {
        clean_request(NULL, NULL, args);
    }

    while (true) {
        got_bytes = recv(args->fd, *buf + ptr, BUFFER_SIZE - ptr, 0);
        if (0 > got_bytes) {
            // The receive was interrupted by delivery of a signal before any data were available; see signal(7).
            if (errno == EINTR) {
                continue;
            }
            clean_request(NULL, *buf, args);
        }

        if (0 == ptr &&  0 == got_bytes) { // empty body case
            clean_request(NULL, *buf, args);
        }

        if (0 == got_bytes) {
            (*buf)[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if (ptr >= BUFFER_SIZE) {

            // DOS_PROTECTION is a switcher used in socket.c, when it is true we close the connection immediately without read the whole message, else we read the entire message and we response with error.
            vlog_ut(1, "Dos Protection %d\n", DOS_PROTECTION);
            args->type_Request = 1;
            //clean_request(NULL, *buf, args);

            if (!DOS_PROTECTION)
                socket_drain_tcp(args->fd);
            return -2;
            //socket_send_error_to_client();
        }
        if (ut_get_line(*buf, ptr)) {
            break;
        }
    }
    return 0;
}

void socket_read_request2(struct ThreadArgs *args, char **buf) {

    int ptr = 0;
    ssize_t got_bytes = 0;

    *buf = calloc(BUFFER_SIZE + 1, sizeof(char));
    if (*buf == NULL) {
        clean_request(NULL, NULL, args);
    }


    got_bytes = recv(args->fd, *buf, BUFFER_SIZE, 0);
    if (0 > got_bytes) {
        clean_request(NULL, *buf, args);
    }
    if (0 == got_bytes) {
        // TODO controllare quando recv return 0
        (*buf)[ptr] = 0; // Terminate string
    }

    // log_ut("socket/socket_read_request %d %d |%s|\n", (*buf)[got_bytes - 1], (*buf)[got_bytes - 2], *buf);

    for (int i = 2; 0 <= i; i--) {
        if (0 != got_bytes && ((*buf)[got_bytes - i] == '\r' || (*buf)[got_bytes - i] == '\n')) {
            (*buf)[got_bytes - i] = '\0';
            break;
        }
    }

    socket_drain_tcp(args->fd);

    log_ut("socket/socket_read_request %ld %ld |%s|\n", got_bytes, strlen(*buf), *buf);
}


int socket_drain_tcp(int fd_client) {
    char bufTMP[BUFFER_SIZE];
    int drain_recv = 0;
    while (true) {
        drain_recv = recv(fd_client, bufTMP, BUFFER_SIZE, 0);
        if (0 > drain_recv) {
            // The receive was interrupted by delivery of a signal before any data were available; see signal(7).
            if (errno == EINTR) {
                continue;
            }
            log_ut("%d \t %s\n", drain_recv, "generic error");
            return -1;
        } else if (drain_recv == 0) {
            break;
        }
        if (ut_get_line(bufTMP, drain_recv)) {
            break;
        }
    }
    return 0;
}

int socket_drain_tcp2(int fd_client) {
    char bufTMP[BUFFER_SIZE];
    int drain_recv = 0;
    while ((drain_recv = recv(fd_client, bufTMP, BUFFER_SIZE, SOCKET_NON_BLOCKING)) > 0) {
        // return mess error
    }
    if (0 > drain_recv) {

        if (drain_recv == EAGAIN) {
        } else {
            log_ut("%d \t %s\n", drain_recv, "gemneric error");
        }
        return -1;
    }
    return 0;
}

int socket_send_message(int fd, char *message_string) {

//    log_ut("socket_send_message: %d, %s", fd, message_string);
    int ret = send(fd, message_string, sizeof(char) * strlen(message_string), 0);
    if (0 > ret) {
        perror("socket.c/socket_send_message: send failed");
        return ret;
    }
    return 0;
}

int socket_send_error_to_client(char *path, char *buf, struct ThreadArgs *args) {
    char *m;
    int err = protocol_response('3', buf, path, "localhost", args->configs.port_number, &m);
    if (err != 0) {
        clean_request(path, buf, args);
    }

    socket_send_message(args->fd, m);
    if (m != NULL) {
        free(m);
    }
    clean_request(path, buf, args);
    return 0;
}

int socket_send_too_long_error_to_client(char *path, char *buf, struct ThreadArgs *args) {
    char mes[50] = {0};
    if ( 0 <= sprintf(mes, "\n3requested path to long\terror.host\t%d\n", args->configs.port_number)){
        socket_send_message(args->fd, mes);
    }
    clean_request(path, buf, args);

    //send(args->fd, m, sizeof(char) * strlen(m), 0);
    return 0;
}

void socket_manage_files(char *path, char *buf, struct ThreadArgs *args) {

    int type_file = file_type(path);
    if (FILES_NOT_EXIST == type_file || type_file == FILES_NOT_PERMITTED) {
        args->type_Request = 1;

        log_ut("socket_manage_files: Il file non esiste");

        socket_send_error_to_client(path, buf, args);
    }

    char code = getGopherCode(path);
    if (code < 0) {
        clean_request(path, buf, args);
    }

    if (FILES_IS_DIRECTORY == type_file) {
        args->type_Request = 1;

        // it's a directory
        log_ut("%s\n", "Sending Directory");
        // TODO che for exit code of print_directory
        print_directory(path, &socket_send_message, args->fd, args->configs.port_number);
    } else if (FILES_IS_REG_FILE == type_file) { // FILES_IS_FILE
        // it's some kind of files
        log_ut("%s\n", "Sending File");

        args->type_Request = 0;

#ifdef _WIN32
        int dim_file_to_send = windows_memory_mapping(args->fd, path);
        //clean_request(path, buf, args);
        if (-1 == dim_file_to_send){
            perror("dim_file_to_send");
            windows_perror();
        }
        printf("HO INVIATO IL FILE %c \n", args->configs.mode_concurrency);
        struct PipeArgs pipeargs1;
        pipeargs1.path = path;
        pipeargs1.ip_client = args->ip_client;
        pipeargs1.dim_file = dim_file_to_send;
        pipeargs1.port = args->configs.port_number;


        //pipe_simple_write_to_pipe(&pipeargs1);

        if (args->configs.mode_concurrency == M_THREAD) {
            PROCESS_INFORMATION pi;

            ZeroMemory(&pi, sizeof(pi));
            pipe_run_process(&pi, configs->mode_concurrency);

            pipe_simple_write_to_pipe(&pipeargs1);

            WaitForSingleObject(&pi.hProcess, INFINITE);

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } else{
            pipe_simple_write_to_pipe(&pipeargs1);
        }



        log_ut("End createProcess");
#endif
#if defined(__unix__) || defined(__APPLE__)

//        struct MemoryMappingArgs *memory_mapping_args = calloc(1, sizeof(struct MemoryMappingArgs));
        struct MemoryMappingArgs memory_mapping_args;

        memory_mapping_args.path = path;
        memory_mapping_args.mode_concurrency = args->configs.mode_concurrency;
        memory_mapping_args.fd = args->fd;

        log_ut("going to linux_memory_mapping\n");
        int map_size = linux_memory_mapping((void *) &memory_mapping_args);

        if (map_size < 0) {
            perror("socket_manage_files/linux_memory_mapping");
            // todo return ?
            return;
        }

        if (LOG_WITH_MULTIPLE_PROCESS) {
            int fd_pipe[2];
            socket_pipe_multiple_process(fd_pipe);
            socket_pipe_log_server(path, args, map_size, fd_pipe[PIPE_WRITE]);
        } else {
            socket_pipe_log_server_single_process(path, args, map_size, global_fd_pipe[PIPE_WRITE]);
        }

#endif

    }
}

int write_to_log(struct PipeArgs *data) {
    FILE *fp_log = fopen("../gopher_log_file.txt", "a");
    if (fp_log == NULL) {
        log_ut("%s", "PERCHÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉ");
        return -1;
    }
    fprintf(fp_log, "%s", "ciaoo");
    log_ut("sono dentro la funzionee");
    fclose(fp_log);

    return 0;

}

int SendFileMapped(int write_fd, char *fileToSend, int fileSize) {
    if (send(write_fd, fileToSend, fileSize, 0) == -1) {
        return -2;
    }
    return 0;
}

/*
int vecchiafork(char *path, char *ip_client, int dim_file_to_send) {
    pid_t child;
    int fd_pipe[2];
    //FILE *fp_log = fopen(LOG_PATH, "a");
    if (pipe(fd_pipe) < 0) {
        perror("pipe");
    }

    child = fork();

    if (child < 0) {
        perror("error in fork");
    } else if (child > 0) {
        close(fd_pipe[0]);
        struct PipeArgs pipeArgs1;
        pipeArgs1.path = path;
        pipeArgs1.ip_client = ip_client;
        pipeArgs1.dim_file = dim_file_to_send;
        write(fd_pipe[1], &pipeArgs1, sizeof(pipeArgs1));
        close(fd_pipe[1]);
    } else if (child == 0) {
        close(fd_pipe[1]);
        write_log("---- child process wrote\n");
        //FILE* fp_fileLog = fopen(LOG_PATH, "w");
        int fd_log = open(LOG_PATH, O_WRONLY | O_APPEND);
        //FILE* fp_filelog= fdopen(fd_log, "a");
        write_log("---- child process open\n");
        if (fd_log == -1) {
            //if (fp_fileLog == NULL){
            write_log("sono bloccato");
            exit(-1);
        }
        //int n;
        struct PipeArgs data;

        //ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
        ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
        write_log("%zu", nread);


        write_log("%zu", nread);

        write_log("---- child process read\n");


        //write_log("\n sono figlio :-> %s\n", data->ip_client);
        write_log("FileName: %s\n", data.path);
        write_log("%d Byte \n", data.dim_file);
        log_ut("IP Client: %s\n", data.ip_client);

         dwrite_log(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file,
                               data.ip_client);
        //int err = fwrite_log(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
        perror("dwrite_log");
        //write(fd_log, "cia", sizeof("cia"));

        //write_log("SONO N %d \n", n);
        close(fd_pipe[0]);

        write_log("---- child process close\n");
        exit(0);
    }
    return 0;
}
*/
/*old version
 *
void socket_read_request(struct ThreadArgs *args, char **buf) {

    int ptr = 0;
    ssize_t got_bytes = 0;

    *buf = calloc(BUFFER_SIZE, sizeof(char));
    if (*buf == NULL) {
        clean_request(NULL, NULL, args);
    }

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
}*/


