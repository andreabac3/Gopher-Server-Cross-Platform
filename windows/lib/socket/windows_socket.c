//
// Created by andrea on 14-Jun-19.
//
#include <winsock2.h>
#include <stdio.h>
#include "windows_socket.h"

// int windows_socket(struct Configs configs) {
int windows_socket() {

    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;

    WSAStartup(MAKEWORD(2, 0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);

    bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
    listen(server, 0);

    printf("Listening for incoming connections...");

    char buffer[1024];
    int clientAddrSize = sizeof(clientAddr);
    if ((client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
        printf("Client connected!");
        recv(client, buffer, sizeof(buffer), 0);
        printf("Client says: ");
        memset(buffer, 0, sizeof(buffer));

        closesocket(client);
        printf("Client disconnected.");
    }
    return 0;
}
