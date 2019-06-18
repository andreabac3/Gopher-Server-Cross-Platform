//
// Created by andrea on 18-Jun-19.
//

#ifndef GOPHERWIN_WINTHREAD_H
#define GOPHERWIN_WINTHREAD_H
#include <windows.h>
DWORD WINAPI ThreadFunc(void* data);
DWORD WINAPI handle_request(void *args);
#endif //GOPHERWIN_WINTHREAD_H

