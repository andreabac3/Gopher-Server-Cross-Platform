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
#include <utils.h>
#include "windows_socket.h"
#include "winThread.h"
#include "definitions.h"
#include "files_interaction.h"
#include "socket.h"


int windows_socket_runner(struct Configs *configs) {
    HANDLE thread;
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
    serverAddr.sin_port = htons(configs->port_number);

    bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr));
    listen(server, 0);

    printf("Listening for incoming connections...");

    // int accept_fd;
    int clientAddrSize = sizeof(clientAddr);


    // new struct for thread/process
    // TODO testare e semplificare
    struct Configs c2;
    struct Configs* n;
    c2.reset_config = NULL ;
    n = &c2;
    ut_clone_configs(configs, n);

    struct ThreadArgs args;
    args.configs = *n;


    FD_ZERO(&read_fds);
    FD_SET(server, &read_fds);
    int n_ready;
    //Windows
    while (MAX_CONNECTIONS_ALLOWED) {
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

            handle_request((PVOID) &args);

            /*if (0 != (thread = CreateThread(NULL, 0, handle_request, (PVOID) &args, 0, NULL))) {
                printf("funziona\n");
            }*/

        }


    }

}

DWORD WINAPI handle_request(void *params) {

    // TODO thread detached
    char *buf;
    char *path;
    struct ThreadArgs *args;
    args = (struct ThreadArgs *) params;

    printf("%s\n", "Running in thread - handle request");
    printf("args: %d\n", args->fd);

    socket_read_request(args, &buf); // fill the buffer with the request

    printf("%s\n", buf);

    socket_resolve_selector(args, buf, &path); // parse the request

    // todo fix resolve_selector come su linux
    printf("full path %s \n", path);
    socket_manage_files(path, buf, args); // send response
    clean_request(path, buf, args);
    return 0;
}



int w_sendFile(int fd_client, char* message_to_send) {
    int bufferSize = 512;
    char buffer[bufferSize];
    int sendPosition = 0;
    int message_len = strlen(message_to_send);
    while(message_len>0){
        int chunkSize = message_len > bufferSize ? bufferSize : message_len;
        memcpy(buffer, message_to_send + sendPosition, chunkSize);
        chunkSize = send(fd_client, buffer, chunkSize, 0);
        // TODO controllare send
        if (chunkSize == -1) { break; }
        message_len -= chunkSize;
        sendPosition += chunkSize;
    }
    return 0;
}

