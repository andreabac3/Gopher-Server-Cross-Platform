//
// Created by valerioneri on 6/13/19.
//

#ifndef GOPHER_LINUX_THREAD_H
#define GOPHER_LINUX_THREAD_H

#include <pthread.h>

int thr_pthread_create(pthread_t *tid, void *(*func)(void *), void *arg);

#endif //GOPHER_LINUX_THREAD_H
