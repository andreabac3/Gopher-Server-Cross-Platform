#include <windows.h>
#include <stdio.h>
#include "windows_utils.h"



int windows_perror(){
    DWORD errorMessageID = GetLastError();
    if (! errorMessageID) {return 0;}
    LPSTR messageBuffer = NULL;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    printf("%s", messageBuffer);
    perror(messageBuffer);
    free(messageBuffer);
    return 0;
}

