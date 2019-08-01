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

#endif

#if defined(__unix__) || defined(__APPLE__)

#include <sys/socket.h>
#include <zconf.h>
#include <pthread.h>

#include <fcntl.h>
#include "linux_memory_mapping.h"
#include "linux_files_interaction.h"
#include "linux_pipe.h"
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
    //int ret = 0;
    if (args->configs.mode_concurrency == M_THREAD) {
        free(args);
        //ExitThread(ret);
        _endthread();
    } else { // mode_concurrency == M_PROCESS
        free(args);
        perror("i processi ancora non sono stati implementati");
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
        int err = close(args->fd);
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
    //printf("Selector string: %s\n", buf);
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
}

int socket_send_message(int fd, char *message_string) {

//    printf("socket_send_message: %d, %s", fd, message_string);
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
        args->type_Request = 1;
        printf("socket_manage_files: Il file non esiste");

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

    char code = getGopherCode(path);
    if (code < 0) {
        clean_request(path, buf, args);
    }

    if (FILES_IS_DIRECTORY == type_file) {
        // it's a directory
        args->type_Request = 1;
        printf("%s\n", "Sending Directory");
        // TODO che for exit code of print_directory
        print_directory(path, &socket_send_message, args->fd, args->configs.port_number);
    } else if (FILES_IS_REG_FILE == type_file) { // FILES_IS_FILE
        // it's some kind of files
        printf("%s\n", "Sending File");
        args->type_Request = 0;

#ifdef _WIN32
        int dim_file_to_send = windows_memory_mapping(args->fd, path);
        //clean_request(path, buf, args);
        printf("HO INVIATO IL FILE\n");
        struct PipeArgs pipeargs1;
        pipeargs1.path = path;
        pipeargs1.ip_client = args->ip_client;
        pipeargs1.dim_file = dim_file_to_send;

        pipe_simple_write_to_pipe(&pipeargs1);
/*
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        if (CreateProcess("C:\\Users\\andrea\\CLionProjects\\gopher5\\gopher-project\\cmake-build-debug\\gopherWinSubProcess.exe",
                          "readPipe", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
            // WaitForSingleObject(pi.hProcess, INFINITE);
        }else{
            perror("createprocess = false");
        }


        // CloseHandle(pi.hThread);
        // CloseHandle(pi.hProcess);
*/


        printf("End createProcess");
        //printf("%d", SendFile(args->fd, fp_FileToSend));int socket_pipe_log_server(char *path, struct ThreadArgs *args, int dim_file_to_send, int *fd_pipe_log)
#endif
#if defined(__unix__) || defined(__APPLE__)

//        struct MemoryMappingArgs *memory_mapping_args = calloc(1, sizeof(struct MemoryMappingArgs));
        struct MemoryMappingArgs memory_mapping_args;

        memory_mapping_args.path = path;
        memory_mapping_args.mode_concurrency = args->configs.mode_concurrency;
        memory_mapping_args.fd = args->fd;

        printf("going to linux_memory_mapping\n");
        int map_size = linux_memory_mapping((void *) &memory_mapping_args);

        if (map_size < 0){
            perror("socket_manage_files/linux_memory_mapping");
            // todo return ?
            return;
        }

//<<<<<<< HEAD
        socket_pipe_log_server(path, args, map_size, fd_pipe);
//=======
//        socket_pipe_log(path, args, map_size);
//>>>>>>> parent of df10553... fixed send pipe log with condition variable

#endif

    }
}

int write_to_log(struct PipeArgs *data) {
    FILE *fp_log = fopen("../gopher_log_file.txt", "a");
    if (fp_log == NULL) {
        printf("%s", "PERCHÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉÉ");
        return -1;
    }
    fprintf(fp_log, "%s", "ciaoo");
    printf("sono dentro la funzionee");
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
        printf("---- child process wrote\n");
        //FILE* fp_fileLog = fopen(LOG_PATH, "w");
        int fd_log = open(LOG_PATH, O_WRONLY | O_APPEND);
        //FILE* fp_filelog= fdopen(fd_log, "a");
        printf("---- child process open\n");
        if (fd_log == -1) {
            //if (fp_fileLog == NULL){
            printf("sono bloccato");
            exit(-1);
        }
        //int n;
        struct PipeArgs data;

        //ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
        ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
        printf("%zu", nread);


        printf("%zu", nread);

        printf("---- child process read\n");


        //printf("\n sono figlio :-> %s\n", data->ip_client);
        printf("FileName: %s\n", data.path);
        printf("%d Byte \n", data.dim_file);
        printf("IP Client: %s\n", data.ip_client);

         dprintf(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file,
                               data.ip_client);
        //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
        perror("dprintf");
        //write(fd_log, "cia", sizeof("cia"));

        //printf("SONO N %d \n", n);
        close(fd_pipe[0]);

        printf("---- child process close\n");
        exit(0);
    }
    return 0;
}
*/
