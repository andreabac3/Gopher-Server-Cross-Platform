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


    HANDLE hMapFile = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 1024, 0);
    if (hMapFile == NULL) {
        _tprintf(TEXT("hMapFile is NULL: last error: %d\n"), GetLastError());
    }

    LPVOID lpMapAddress = MapViewOfFile(hMapFile,            // handle to
            // mapping object
                                        FILE_MAP_READ, // read/write
                                        0,                   // high-order 32
            // bits of file
            // offset
                                        0,      // low-order 32
            // bits of file
            // offset
                                        1024);      // number of bytes
    // to map
    char *pBuf = (char *) lpMapAddress;
    printf("SONO PBUFFFF: %s\n", pBuf);
    //free(pBuf);
    w_sendFile(fd_client, pBuf);


    if (lpMapAddress == NULL) {
        _tprintf(TEXT("lpMapAddress is NULL: last error: %d\n"), GetLastError());
        return -1;
    }
    printf("Read message %s", lpMapAddress);


    BOOL bFlag = UnmapViewOfFile(lpMapAddress);
    bFlag = CloseHandle(hMapFile); // close the file mapping object

    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the mapping object!"),
                 GetLastError());
        return -1;
    }

    bFlag = CloseHandle(hFile);   // close the file itself

    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the file!"),
                 GetLastError());
        return -1;
    }
    return 0;
}

