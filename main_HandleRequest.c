#include <stdio.h>
#include <windows.h>

#ifdef _IA64_
#pragma warning(disable:4127 4706 4267)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
//#include <wspiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows_utils.h>
#include <socket.h>
#include <windows_pipe.h>
#include "windows_socket.h"


int main(int argc, char *argv[]) {
    printf("I am handle request.exe, my args are: %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
    struct Configs h_configs;
    h_configs.port_number = atoi(argv[1]);
    h_configs.mode_concurrency = argv[3][0];
    if (strlen(argv[2]) > BUFFER_SIZE) { return -1; }
    strcpy(h_configs.root_dir, argv[2]);
    printf("I am handle request.exe, my args are: %s %d %s %d", argv[0], h_configs.port_number, h_configs.root_dir,
           h_configs.mode_concurrency);

    struct ThreadArgs h_args;
    h_args.configs = h_configs;
    h_args.ip_client = argv[0];
    h_args.hostname = argv[4];
    printf("\nSONO ARGV4 %s \n", argv[4]);
    if (strlen(argv[4]) < BUFFER_SIZE) {
        strcpy(ip_buffer, argv[4]);
    }
    PROCESS_INFORMATION pi;

    ZeroMemory(&pi, sizeof(pi));
    pipe_run_process(&pi, h_configs.mode_concurrency);


    WSADATA wsaData;
    int nStatus;
    if ((nStatus = WSAStartup(0x202, &wsaData)) != 0) {
        fprintf(stderr, "\nWinsock 2 DLL initialization failed: %d\n", nStatus);
        WSACleanup();
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        exit(-1);
    }


    HANDLE hPipe;
    WSAPROTOCOL_INFO ProtocolInfo;
    DWORD dwRead;
    SOCKET sockDuplicated;


    hPipe = CreateFile(TEXT("\\\\.\\pipe\\PipeHandleRequest"),
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        windows_perror();
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        exit(-1);
    }


    if (!ReadFile(hPipe, &ProtocolInfo, sizeof(WSAPROTOCOL_INFO) - 1, &dwRead, NULL)) {
        windows_perror();
        CloseHandle(hPipe);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        exit(-1);
    }
    sockDuplicated = WSASocket(FROM_PROTOCOL_INFO,
                               FROM_PROTOCOL_INFO,
                               FROM_PROTOCOL_INFO,
                               &ProtocolInfo,
                               0,
                               0);

    if (sockDuplicated == INVALID_SOCKET) {
        windows_perror();
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(hPipe);
        exit(-1);
    }


    CloseHandle(hPipe);

    printf("get duplicated socket");
    h_args.fd = sockDuplicated;


    handle_request(&h_args);
    WaitForSingleObject(&pi.hProcess, INFINITE);
    // TODO MOD PROCESSO: questa funzione non chiude o termina l'invio del file a meno di fare ctrl+c dal client.

    //CloseHandle(pipe_read);
    CloseHandle(hNamedPipe);


    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    /*socket_send_message(sockDuplicated, "Risposta da processo");
    windows_perror();

    int err = closesocket(sockDuplicated);
    if (err != 0) {
        windows_perror();
        perror("Close in clean request");
    }

    // Got duplicated socket


    printf("child finished");*/
    return 0;
}


// other stuf

//char *DEFAULT_PORT                  = "8765";
//const char FILE_MAPPING_BASE_NAME[] = "/MAPPED_FILE/WSADuplicateSocket";
const char PARENT[] = "parent";
const char CHILD[] = "child";
//static int nChildProcCount          = 0;
//SOCKET gsListen                     = INVALID_SOCKET;
//SOCKET gsAccept                     = INVALID_SOCKET;
HANDLE ghParentFileMappingEvent = NULL;
HANDLE ghChildFileMappingEvent = NULL;
HANDLE ghMMFileMap = NULL;

// Once the protocol information is set by the
// parent in the memory mapped file, the child
// is ready to use it to duplicate the socket.
SOCKET GetSocket(char *szFileMappingObj) {
    WSAPROTOCOL_INFO ProtocolInfo;
    SOCKET sockDuplicated = INVALID_SOCKET;
    char szParentEventName[MAX_PATH];
    char szChildEventName[MAX_PATH];

    sprintf(szParentEventName, "%s%s", szFileMappingObj, PARENT);
    sprintf(szChildEventName, "%s%s", szFileMappingObj, CHILD);

    // Open the events
    if ((ghParentFileMappingEvent = OpenEvent(SYNCHRONIZE, FALSE, szParentEventName)) == 0) {
        fprintf(stderr, "OpenEvent() failed: %d\n", GetLastError());
        return INVALID_SOCKET;
    }

    if ((ghChildFileMappingEvent = OpenEvent(SYNCHRONIZE, FALSE, szChildEventName)) == 0) {
        fprintf(stderr, "OpenEvent() failed: %d\n", GetLastError());
        CloseHandle(ghParentFileMappingEvent);
        ghParentFileMappingEvent = NULL;
        return INVALID_SOCKET;
    }

    // Wait for the parent to signal that the protocol info
    // is ready to be accessed
    if (WaitForSingleObject(ghParentFileMappingEvent, 2000) == WAIT_FAILED) {
        fprintf(stderr, "WaitForSingleObject() failed: %d\n", GetLastError());
        //DoCleanup();
        exit(1);
    }

    ghMMFileMap = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
                                  FALSE,
                                  szFileMappingObj);

    if (ghMMFileMap != NULL) {
        LPVOID lpView = MapViewOfFile(ghMMFileMap,
                                      FILE_MAP_READ | FILE_MAP_WRITE,
                                      0, 0, 0);

        if ((BYTE *) lpView != NULL) {
            int nStructLen = sizeof(WSAPROTOCOL_INFO);

            memcpy(&ProtocolInfo, lpView, nStructLen);
            UnmapViewOfFile(lpView);

            // Duplicate the socket based on the protocol
            // information stored in the memory mapped file.
            sockDuplicated = WSASocket(FROM_PROTOCOL_INFO,
                                       FROM_PROTOCOL_INFO,
                                       FROM_PROTOCOL_INFO,
                                       &ProtocolInfo,
                                       0,
                                       0);

            // Signal the parent the we are done
            // with the mapped file
            SetEvent(ghChildFileMappingEvent);
        } else
            fprintf(stderr, "MapViewOfFile() failed: %d\n", GetLastError());
    } else
        fprintf(stderr, "CreateFileMapping() failed: %d\n", GetLastError());

    if (ghParentFileMappingEvent != NULL) {
        CloseHandle(ghParentFileMappingEvent);
        ghParentFileMappingEvent = NULL;
    }

    if (ghChildFileMappingEvent != NULL) {
        CloseHandle(ghChildFileMappingEvent);
        ghChildFileMappingEvent = NULL;
    }

    if (ghMMFileMap != NULL) {
        CloseHandle(ghMMFileMap);
        ghMMFileMap = NULL;
    }

    return sockDuplicated;
}

void DoChild(char *pszChildFileMappingObj) {
    SOCKET sockDuplicated = INVALID_SOCKET;
    DWORD dwProcID;
    char szBuf[MAX_PATH + 1];
    WSABUF wsaBuf;
    DWORD dwReceived = 0;
    DWORD dwFlags = 0;
    int nStatus;

    dwProcID = GetCurrentProcessId();

    printf("Child process %lu started ...\n", dwProcID);

    if ((sockDuplicated = GetSocket(pszChildFileMappingObj))
        != INVALID_SOCKET) {
        while (TRUE) {
            szBuf[0] = '\0';
            wsaBuf.len = MAX_PATH;
            wsaBuf.buf = (char *) szBuf;

            // Receive the data sent by the other side.
            nStatus = WSARecv(sockDuplicated,
                              &wsaBuf,
                              1,
                              &dwReceived,
                              &dwFlags,
                              (LPWSAOVERLAPPED) NULL,
                              0);

            if (nStatus == 0) // success
            {
                if (dwReceived == 0) // Client closed connection
                {
                    fprintf(stderr, "Client closed connection\n");
                    break;
                } else {
                    // Print whatever is received
                    szBuf[dwReceived] = '\0';
                    printf("%s", szBuf);
                }
            } else // SOCKET_ERROR
            {
                fprintf(stderr, "WSARecv() failed: %d\n", WSAGetLastError());
                break;
            }
        }

        //DoGracefulShutdown(sockDuplicated);
        sockDuplicated = INVALID_SOCKET;
    } else
        fprintf(stderr, "\nChild socket cannot be obtained: %d\n",
                WSAGetLastError());

    printf("\nChild process %lu has finished!!!\n", dwProcID);

    printf("Press any key to continue ...");
    getchar();
}
