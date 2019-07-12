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
#include "socket.h"
#include "windows_socket.h"
#include "winThread.h"
#include "definitions.h"

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

    char *m2 = "\nbienvenue\n";
    int serr = send(args->fd, m2, sizeof(char) * strlen(m2), 0);
    if (serr != 0) {
        printf("%s\n", "Unable to send Socket");
        perror("Send");
    }

    printf("%s\n", "Running in thread - handle request");

    printf("args: %d\n", args->fd);

    /*Code for gopher protocol*/
    int run = 1;
    int ptr = 0;
    ssize_t got_bytes = 0;
    char *buf = calloc(BUFFER_SIZE, sizeof(char));

    //recv()

    while (run) {
        got_bytes = recv(args->fd, buf + ptr, BUFFER_SIZE - ptr, 0);
        if (got_bytes <= 0) {
            buf[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if (get_line(buf, ptr)) {
            break;
        }
    }

    printf("Selector string: %s\n", buf);

    char *path = resolve_selector(args->configs.root_dir, NULL, buf);
    // todo fix resolve_selector come su linux
    printf("full path %s \n", path);

    /* if (!file_exist(path)) {
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
             //pthread_exit(&ret);
         }
     }*/

    char code = getGopherCode(path);
    if (code < 0) {
        // error
        //linux_sock_send_error(args->fd);
        close(args->fd);
        int ret = 0;
        //pthread_exit(&ret);
    }
    printf("Code: %d\n", code);
    if (code == '1') {
        // it's a directory
        printf("%s\n", "Directory");
        char *m = "Directory\n";
        send(args->fd, m, sizeof(char) * strlen(m), 0);
        print_directory(path, &linux_sock_send_message, (int *) &(args->fd));
    } else {
        // it's some kind of files
        printf("%s\n", "filesssss");
        FILE* fp_FileToSend = sendFileToClient(path);
        int remain_data = fsize(fp_FileToSend);
        // int, int, off_t, off_t *, struct sf_hdtr *, int);
        //sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
        printf("%d", SendFile(args->fd, fp_FileToSend, remain_data));
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


int windows_socket(struct Configs configs) {

    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;

    if (0 != WSAStartup(MAKEWORD(2, 2), &WSAData)) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(configs.port_number);

    bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
    listen(server, 0);

    printf("Listening for incoming connections...");

    char request_path[1024] = {0}; // memset per velocità.

    // int accept_fd;
    int clientAddrSize = sizeof(clientAddr);

    struct ThreadArgs args;
    args.configs = configs;

    //Windows
    while (INVALID_SOCKET != (client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize))) {
        printf("Client connected!\n");
        printf("IP address is: %s\n", inet_ntoa(clientAddr.sin_addr));
        //int* req_fd = (int *) client;

        args.fd = client;
        HANDLE thread;
        if (0 != (thread = CreateThread(NULL, 0, handle_request, (PVOID) &args, 0, NULL))) {
            printf("funziona\n");
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
    }

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
