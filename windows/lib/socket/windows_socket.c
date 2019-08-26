#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include <unistd.h>
#include <errno.h>
#include <ws2tcpip.h>
#include <excpt.h>
#include <stdbool.h>
#include <libgen.h>
#include <tchar.h>
#include "utils.h"
#include "windows_socket.h"
#include "winThread.h"
#include "definitions.h"
#include "files_interaction.h"
#include "socket.h"
#include "protocol.h"
#include "windows_pipe.h"
#include "windows_protocol.h"
#include "windows_utils.h"

int end_server(SOCKET fd) {
    shutdown(fd, 2);
    closesocket(fd);
    return 0;
}

void run_concurrency(struct ThreadArgs *args, SOCKADDR_IN clientAddr, SOCKET client) {

    HANDLE thread;


    args->ip_client = inet_ntoa(clientAddr.sin_addr);
    printf("Client connected!\n");
    printf("IP address is: %s\n", args->ip_client);
    args->fd = client;
    if (args->configs.mode_concurrency == M_THREAD) {
        // handle_request((PVOID) &args);
        // TODO cambiare con _beginthread
        if (0 != (thread = CreateThread(NULL, 0, handle_request, (PVOID) &args, 0, NULL))) {
            printf("funziona\n");
        }
        CloseHandle(thread);
    } else if (args->configs.mode_concurrency == M_PROCESS) {

    } else {
        printf("errore in mod concurrency");
        exit(-1);
    }

}

void run_process(struct ThreadArgs *args, SOCKADDR_IN *clientAddr, SOCKET client) {

    // Create named pipe

    DWORD dwWritten;


    /*hNamedPipe = CreateFile(TEXT("\\\\.\\pipe\\PipeHandleRequest"),
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE){
       return;
    }*/

    // Create Process
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    // process
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char cmd_child[BUFFER_SIZE * 2] = {0};
    snprintf(cmd_child, BUFFER_SIZE * 2, "%s %d \"%s\" %d", inet_ntoa(clientAddr->sin_addr), configs->port_number,
             configs->root_dir, configs->mode_concurrency);
    fprintf(stderr, "cmd child %s\n", cmd_child);

    if (FALSE == CreateProcess("gopherWinHandleRequestProcess.exe", cmd_child, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL,
                      NULL, &si, &pi)) {
        return;
    }

    WSAPROTOCOL_INFO ProtocolInfo;

    BOOL err = WSADuplicateSocketA(client, pi.dwProcessId, &ProtocolInfo);



    if (err) {
        fprintf(stderr, "WSADuplicateSocket(): failed. Error = %d, %s\n", WSAGetLastError()), windows_perror();
        //DoCleanup();
        exit(1);
    }





    if (ConnectNamedPipe(hNamedPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
    {
        if (!WriteFile(hNamedPipe,
                       &ProtocolInfo,
                       sizeof(ProtocolInfo),   // = length of string + terminating '\0' !!!
                       &dwWritten,
                       NULL)) {
            windows_perror();
        }

        DisconnectNamedPipe(hNamedPipe);
        //CloseHandle(hNamedPipe);
    }

    printf("SONO QUIIIII OHH");

    WaitForSingleObject(pi.hProcess, INFINITE);

    if (closesocket(client) != 0) {
        perror("Close in clean request");
    }



    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    printf("prima di sleep");



    printf("Dopo close socket di sleep");

}

int windows_socket_runner(struct Configs *configs) {
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
    struct Configs *n;
    c2.reset_config = NULL;
    n = &c2;
    ut_clone_configs(configs, n);

    struct ThreadArgs args;
    args.configs = *n;

    FD_ZERO(&read_fds);
    FD_SET(server, &read_fds);
    int n_ready;
    //Windows
    struct timeval timeout;
    timeout.tv_sec = SOCK_START_TIMEOUT;
    timeout.tv_usec = SOCK_START_TIMEOUT;
    while (MAX_CONNECTIONS_ALLOWED) {
        memcpy(&working_set, &read_fds, sizeof(read_fds));
        printf("\n%s %d\n", "server ->", server);
        // rc =
        // # richieste di connessione
        if ((n_ready = select(server + 1, &working_set, NULL, NULL, &timeout)) < 0) {
            if (errno == EINTR) continue;
            else {
                windows_perror();
                perror("select errno == EINTR");
                return -1;
            }
        }

        if (n_ready == 0) {
            // printf("  select() timed out.  End program.\n");
            if (configs->reset_config != NULL) {
                //end_server(fd_server);
                // printf("Reset socket break\n");
                // printf("SONO SHUTDOWN %d\n", end_server(server));
                return -1;
            }

            printf("Reset socket continue %ls \n", configs->reset_config);
            timeout.tv_sec = SOCK_LOOP_TIMEOUT;
            timeout.tv_usec = SOCK_LOOP_TIMEOUT;
            continue;
        }

        if (FD_ISSET(server, &working_set)) { /* richiesta proveniente da client TCP */
            if ((client = accept(server, (SOCKADDR *) &clientAddr, &clientAddrSize)) < 0) {
                if (errno == EINTR) continue;
                else {
                    perror("accept");
                    continue;
                }
            }
            // todo run concurr
            //run_concurrency(&args, clientAddr, client);
            HANDLE thread;


            args.ip_client = inet_ntoa(clientAddr.sin_addr);
            printf("Client connected!\n");
            printf("IP address is: %s\n", args.ip_client);
            args.fd = client;
            if (args.configs.mode_concurrency == M_THREAD) {
                // handle_request((PVOID) &args);
                // TODO cambiare con _beginthread
                if (0 != (thread = CreateThread(NULL, 0, handle_request, (PVOID) &args, 0, NULL))) {
                    printf("funziona\n");
                }
                CloseHandle(thread);
            } else if (args.configs.mode_concurrency == M_PROCESS) {

                run_process(&args, &clientAddr, client);

            } else {
                printf("errore in mod concurrency");
                exit(-1);
            }
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
    printf("PRIMA DI CLEAN");

    clean_request(path, buf, args);
    printf("DOPO CLEAN");
    return 0;
}


DWORD WINAPI w_sendFile(PVOID args) {

    struct sendArgs *send_args = (struct sendArgs *) args;
    int fd_client = send_args->fd;
    char *message_to_send = send_args->buff;

    int bufferSize = 512;
    char buffer[bufferSize];
    int sendPosition = 0;
    int message_len = strlen(message_to_send);
    while (message_len > 0) {
        int chunkSize = message_len > bufferSize ? bufferSize : message_len;
        memcpy(buffer, message_to_send + sendPosition, chunkSize);
        chunkSize = send(fd_client, buffer, chunkSize, 0);
        if (chunkSize == -1) { break; }
        message_len -= chunkSize;
        sendPosition += chunkSize;
    }
    return 0;
}

int blackListFile(char *baseDir, char *pathFile, char *black_listed_file) {
    char *base_name = basename(pathFile);
    if (strcmp(black_listed_file, base_name) != 0) {
        return 0;
    }
    TCHAR **lppPart = {NULL};

    char base_dir_abs[2048] = {0};
    int ret = GetFullPathNameA(baseDir, 2047, base_dir_abs, lppPart);
    if (ret == 0) {
        return -1;
    }
    char path_file_abs[2048] = {0};
    ret = GetFullPathNameA(pathFile, 2047, path_file_abs, lppPart);
    if (ret == 0) {
        return -1;
    }

    char *real_base_dir = dirname(base_dir_abs);
    char *path_file = dirname(path_file_abs);


    if (strcmp(real_base_dir, path_file) != 0) {
        return 0;

    }
    return 1;
}

