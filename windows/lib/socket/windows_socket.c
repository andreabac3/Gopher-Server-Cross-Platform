//
// Created by andrea on 14-Jun-19.
//
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include <unistd.h>
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

DWORD WINAPI handle_request(void *params) {
//    int * fd = (int *)args;
//    SOCKET* client = (SOCKET*) args;

    struct ThreadArgs * args;
    args = (struct ThreadArgs*) params;

    char* m2 = "\nbienvenue\n";
    send(*args->fd, m2, sizeof(char) * strlen(m2), 0);

    printf("%s\n", "Running in thread - handle request");

    printf("args: %d\n", *args->fd);

//    int err = shutdown(*client, SD_BOTH);
    int err = closesocket(*args->fd);
    printf("%s %d\n", "Close Socket return", err);
    if (err != 0){
        printf("%s\n", "Unable to close Socket");
    }
    printf("Client disconnected.");


    /*int run = 1;
    int ptr = 0;
    ssize_t got_bytes = 0;
    char * buf = malloc(BUFFER_SIZE);
    while( run ) {
        got_bytes = read( *fd, buf + ptr, BUFFER_SIZE - ptr);
        if(got_bytes <= 0){
            buf[ptr] = 0; // Terminate string
            break;
        }
        ptr += got_bytes;
        if( get_line(buf, ptr) ){
            break;
        }
    }
    printf("sono qui %s\n", buf);


//    send(*fd, "ciao", 5, 0);

    printf("%s\n", "Responding");
    char* m = "bienvenue\n";
    send(*fd, m, sizeof(char) * strlen(m), 0);
    printf("%s\n", "Rresponded!");

    close(*fd);*/
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
        int* req_fd = (int *) client;

        args.fd = (int *) &client;
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
