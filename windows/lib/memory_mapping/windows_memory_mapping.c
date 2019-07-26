//
// Created by andrea on 23-Jul-19.
//

#include <afxres.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <stdint.h>
#include "windows_memory_mapping.h"
#include "windows_socket.h"
int windows_memory_mapping(int fd_client, char *path) {
    HANDLE hFile = CreateFile(path,                // name of the write
                              GENERIC_READ,          // open for writing
                              0,                      // do not share
                              NULL,                   // default security
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);                  // no attr. template
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("hFile is NULL\n"));
        return -1;
    }

    // TODO modificare 1024 con dim reale
    HANDLE hMapFile = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 1024, 0);
    if (hMapFile == NULL) {
        _tprintf(TEXT("hMapFile is NULL: last error: %d\n"), GetLastError());
        return -1;
    }

    LPVOID lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ,  0, 0, 1024);
    // to map
    char *pBuf = (char *) lpMapAddress;
    // printf("SONO PBUFFFF: %s\n", pBuf);
    //free(pBuf);
    printf("%d\n", &hFile);
    printf("%d\n", pBuf);
    printf("%s\n", "è colpa di windows");
    // todo fix send filke
    // w_sendFile(fd_client, pBuf);


    if (lpMapAddress == NULL) {
        _tprintf(TEXT("lpMapAddress is NULL: last error: %d\n"), GetLastError());
        return -1;
    }
    // printf("Read message %s", lpMapAddress);


    BOOL bFlag = UnmapViewOfFile(lpMapAddress);
    bFlag = CloseHandle(hMapFile); // close the file mapping object
    printf("%s\n", "è colpa di windows");

    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the mapping object!"),
                 GetLastError());
        return -1;
    }
    printf("%s\n", "è colpa di windows");


    bFlag = CloseHandle(hFile);   // close the file itself
    printf("%s\n", "è colpa di windows");


    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the file!"),
                 GetLastError());
        return -1;
    }
    return sizeof(pBuf);
}

