//
// Created by andrea on 18-Jun-19.
//
#include <stdio.h>

#include "winThread.h"

DWORD WINAPI ThreadFunc(void* data) {
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    printf("ciao");
    // Do stuff.  This will be the first function called on the new thread.
    // When this function returns, the thread goes away.  See MSDN for more details.
    return 0;
}
