#include "linux_thread.h"

int thr_pthread_create(pthread_t *tid, void *(*func)(void *), void *arg){

    pthread_create(tid, NULL, func, arg);
    return 0;
}
