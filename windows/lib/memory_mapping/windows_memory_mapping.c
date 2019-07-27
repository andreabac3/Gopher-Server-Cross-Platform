//
// Created by andrea on 23-Jul-19.
//

#include <afxres.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <stdint.h>
#include <assert.h>
#include "windows_memory_mapping.h"
#include "windows_socket.h"

long windows_GetFileSize(const TCHAR *fileName) {
    BOOL fOk;
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;

    if (NULL == fileName)
        return -1;

    fOk = GetFileAttributesEx(fileName, GetFileExInfoStandard, (void *) &fileInfo);
    if (!fOk)
        return -1;
    assert(0 == fileInfo.nFileSizeHigh);
    return (long) fileInfo.nFileSizeLow;
}

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

    long size_file = windows_GetFileSize(path);
    if (size_file <= 0) {
        CloseHandle(hFile);
        return -1;
    }


    // TODO modificare 1024 con dim reale
    HANDLE hMapFile = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, size_file, 0);
    if (hMapFile == NULL) {
        _tprintf(TEXT("hMapFile is NULL: last error: %d\n"), GetLastError());
        CloseHandle(hFile);

        return -1;
    }

    LPVOID lpMapAddress = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, size_file);

    if (NULL == lpMapAddress) {
        _tprintf(TEXT("MapViewOfFile is NULL: last error: %d\n"), GetLastError());
        CloseHandle(hFile);
        UnmapViewOfFile(lpMapAddress);
        CloseHandle(hMapFile);
        return -1;
    }
    // to map
    char *pBuf = (char *) lpMapAddress;
    struct sendArgs memory_args;
    memory_args.buff = pBuf;
    memory_args.fd = fd_client;
    HANDLE thread;
    if (0 != (thread = CreateThread(NULL, 0, w_sendFile, (PVOID) &memory_args, 0, NULL))) {
        printf("funziona\n");
    }
    WaitForSingleObject(thread, 0);
    // w_sendFile(fd_client, pBuf);


    BOOL bFlag = UnmapViewOfFile(lpMapAddress);
    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the mapping object!"),
                 GetLastError());
    }
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

