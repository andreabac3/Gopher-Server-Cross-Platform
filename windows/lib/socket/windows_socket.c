//
// Created by andrea on 14-Jun-19.
//
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include <unistd.h>
#include <protocol.h>
#include <windows_protocol.h>
#include <errno.h>
#include <ws2tcpip.h>
#include "socket.h"
#include "windows_socket.h"
#include "winThread.h"
#include "definitions.h"
#include "files_interaction.h"


int get_line(char *buf, size_t size) {
    char *pos = memchr(buf, '\n', size);
    if (pos != NULL) {
        *(pos - 1) = 0;
        return 1;
    }
    return 0;
}

#define BUFFER_SIZE 1024

void linux_sock_send_message(int *fd, char *error) {

    printf("linux_sock_send_message: %d, %s", *fd, error);
    send(*fd, error, sizeof(char) * strlen(error), 0);
    perror("linux_sock_send_message");
}

DWORD WINAPI handle_request(void *params) {
//    int * fd = (int *)args;
//    SOCKET* client = (SOCKET*) args;
    // TODO thread detached
    struct ThreadArgs *args;
    args = (struct ThreadArgs *) params;

    char *m2 = "\niBienvenue\n";
    int serr = send(args->fd, m2, sizeof(char) * strlen(m2), 0);
    if (serr != 0) {
        printf("%s %d\n", "Unable to send Socket", WSAGetLastError());
        perror("Send");
    }

    printf("%s\n", "Running in thread - handle request");

    printf("args: %d\n", args->fd);

    /*Code for gopher protocol*/
    int run = 1;
    int ptr = 0;
    ssize_t got_bytes = 0;
    char *buf = calloc(BUFFER_SIZE, sizeof(char));

    if (buf == NULL) {
        printf("errore calloc in windows_socket, per buffer_size");
        int err = closesocket(args->fd);
        if (err != 0) {
            printf("%s", "errore in windows_socket.c/closesocket after got bytes < 0");
        }
        return -1;
    }
    //recv()

    while (run) {
        got_bytes = recv(args->fd, buf + ptr, BUFFER_SIZE - ptr, 0);
        if (0 > got_bytes) {
            int err = closesocket(args->fd);
            if (err != 0) {
                printf("%s %d", "errore in windows_socket.c/closesocket after got bytes < 0", WSAGetLastError());
            }
            return -1;
        }
        if (0 == got_bytes) {
            buf[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if (get_line(buf, ptr)) {
            break;
        }
    }

    printf("Selector string: %s\n", buf);
    char *path;
    int ret_resolve_relector = resolve_selector(args->configs.root_dir, &path, buf);

    // todo fix resolve_selector come su linux
    printf("full path %s \n", path);

    if (!file_exist(path)) {
        printf("SEND: Il file non esiste");

        // tell to client that file do not exists
        char *m;
        int err = protocol_response('3', buf, "/path/", "localhost", args->configs.port_number, &m);
        if (err != 0) {
            // TODO controlli protocol response , se c'è bisogno di free ecc
            closesocket(args->fd);
            ExitThread(-1);
            //linux_sock_send_error(args->fd);
            // No need to free m, because calloc crashed.
        } else {
            printf("%s", m);
            send(args->fd, m, sizeof(char) * strlen(m), 0);
            free(m);
        }
        closesocket(args->fd);
        int ret = 1;
        ExitThread(0);
    }

    // Todo check if file or directory
    char code = getGopherCode(path);
    if (code < 0) {
        // error
        //linux_sock_send_error(args->fd);
        closesocket(args->fd);

        //int ret = 0;
        //pthread_exit(&ret);
    }
    printf("Code: %d\n", code);
    if (code == '1') {
        // it's a directory
        printf("%s\n", "iDirectory");
        char *m = "iDirectory\n";
        send(args->fd, m, sizeof(char) * strlen(m), 0);
        print_directory(path, &linux_sock_send_message, (int *) &(args->fd));
    } else {
        // it's some kind of files
        printf("%s\n", "filesssss");
        FILE *fp_FileToSend = fopen(path, "r");
        if (fp_FileToSend == NULL) {
            fprintf(stderr, "Error opening file --> %s", strerror(errno));
            closesocket(args->fd);
            return -1;
        }
        //FILE *fp_FileToSend = sendFileToClient(path);
        int remain_data = fsize(fp_FileToSend);
        // int, int, off_t, off_t *, struct sf_hdtr *, int);
        //sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
        printf("%d", SendFile(args->fd, fp_FileToSend, remain_data));
        fclose(fp_FileToSend);
    }

    /*End code*/

    //int err = shutdown(args->fd, SD_BOTH);
    int err = closesocket(args->fd);
//    int err = close(args->fd);

    printf("%s %d\n", "Close Socket return", err);
    if (err != 0) {
        printf("%s\n", "Unable to close Socket");
        perror("Close");
    }
    printf("Client disconnected.");

    return 0;

}


int build_fd_sets(SOCKET socket, fd_set *read_fds) {
    FD_ZERO(read_fds);
    FD_SET(STDIN_FILENO, read_fds);
    FD_SET(socket, read_fds);

    return 0;
}


int windows_socket(struct Configs configs) {

    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    /* socket */
    fd_set read_fds;
    fd_set working_set;

    if (0 != WSAStartup(MAKEWORD(2, 2), &WSAData)) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return -1;
    }

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > server) {
        printf("Errore creazione socket");
        perror("Server = socket ");
    }

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(configs.port_number);

    bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
    listen(server, 0);

    printf("Listening for incoming connections...");

    // int accept_fd;
    int clientAddrSize = sizeof(clientAddr);

    struct ThreadArgs args;
    args.configs = configs;


    FD_ZERO(&read_fds);
    FD_SET(server, &read_fds);
    int n_ready;
    //Windows
    while (TRUE) {
        memcpy(&working_set, &read_fds, sizeof(read_fds));
        printf("\n%s %d\n", "server ->", server);
        // rc =
        // # richieste di connessione
        if ((n_ready = select(server + 1, &working_set, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) continue;
            else {
                perror("select");
                exit(1);
            }
        }

        if (n_ready == 0) {
            printf("  select() timed out.  End program.\n");
            perror("select");
            exit(1);
        }

        if (FD_ISSET(server, &working_set)) { /* richiesta proveniente da client TCP */
            if ((client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize)) < 0) {
                if (errno == EINTR) continue;
                else {
                    perror("accept");
                    exit(1);
                }
            }
            printf("Client connected!\n");
            printf("IP address is: %s\n", inet_ntoa(clientAddr.sin_addr));
            args.fd = client;
            HANDLE thread;
            if (0 != (thread = CreateThread(NULL, 0, handle_request, (PVOID) &args, 0, NULL))) {
                printf("funziona\n");
            }

        }


    }
    /*recv(client, request_path, sizeof(request_path), 0);
    char* m2 = "\nBENVENUTI nel server gopher\n";
    send(client, m2, sizeof(char) * strlen(m2), 0);

    printf("Client says{ \n%s\n}\n fine", request_path);
    memset(request_path, 0, sizeof(request_path));

    int err = shutdown(client, SD_BOTH);
    printf("%s %d\n", "Close Socket return", err);
    if (err != 0){
        printf("%s\n", "Unable to close Socket");
    }
    printf("Client disconnected.");*/


//Backup Windows
/*if (INVALID_SOCKET != (client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize))) {
    printf("Client connected!\n");
    printf("IP address is: %s\n", inet_ntoa(clientAddr.sin_addr));
    int* req_fd = (int *) client;
    HANDLE thread;
    if (0 != (thread = CreateThread(NULL, 0, handle_request, (PVOID) client, 0, NULL))) {
        printf("funziona\n");
    }
    recv(client, request_path, sizeof(request_path), 0);
    char* m2 = "\nBENVENUTI nel server gopher\n";
    send(client, m2, sizeof(char) * strlen(m2), 0);

    printf("Client says{ \n%s\n}\n fine", request_path);
    memset(request_path, 0, sizeof(request_path));

    int err = closesocket(client);
    printf("%s %d\n", "Close Socket return", err);
    if (err != 0){
        printf("%s\n", "Unable to close Socket");
    }
    printf("Client disconnected.");
}*/
    return 0;
}
