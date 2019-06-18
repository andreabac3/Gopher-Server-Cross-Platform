//
// Created by andrea on 14-Jun-19.
//
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include "windows_socket.h"


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

    char buffer[1024];
    int clientAddrSize = sizeof(clientAddr);
    if (INVALID_SOCKET != (client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize))) {
        printf("Client connected!\n");
        printf("IP address is: %s\n", inet_ntoa(clientAddr.sin_addr));

        recv(client, buffer, sizeof(buffer), 0);
        char *messaggio = "\nHello World from the server\n";
        int retval = send(client, messaggio, sizeof(char) * strlen(messaggio), 0);
        printf("Client says{ \n%s\n}\n", buffer);
        memset(buffer, 0, sizeof(buffer));

        closesocket(client);
        printf("Client disconnected.");
    }
    return 0;
}
