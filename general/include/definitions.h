//#pragma once
#ifndef GOPHER_DEFINITIONS_H
#define GOPHER_DEFINITIONS_H

// START FILES definition of file types
#define FILES_NOT_EXIST 0
#define FILES_IS_DIRECTORY 1
#define FILES_IS_REG_FILE 2
#define FILES_IS_UNMANAGED_FILE 3
// END FILES definition of file types


#define CONNECTION_QUEUE 500
#define MAX_CONNECTIONS_ALLOWED 1
#define BUFFER_SIZE 1024
#if defined(__unix__) || defined(__APPLE__)
#define OS_SEPARATOR '/'

#endif
#ifdef _WIN32
#define OS_SEPARATOR '\\'
#endif


#include <sys/stat.h>

struct Configs {
    unsigned int port_number;
    char mode_concurrency;
    char *root_dir;
    int useOPTARG;
};

#if defined(__unix__) || defined(__APPLE__)
struct ThreadArgs {
    int fd;
    struct Configs configs;
};
#endif
#ifdef _WIN32
#include <afxres.h>
struct ThreadArgs {
    SOCKET fd;
    struct Configs configs;
};
#endif


/*enum item_types {
    ITEM_FILE='0',
    ITEM_DIR='1',
    ERROR='3',
    ITEM_ARCHIVE='5',
    ITEM_BINARY='9',
    ITEM_IMAGE='I',
    ITEM_INFO='i',
    ITEM_GIF='g',
    ITEM_PNG='p',
    ITEM_PDF='d',
    NO_ITEM=0
};*/


struct menu_item {
    char type;
    char *display;
    char *selector;
    char *host;
    unsigned int port;
    char delimiter;
};

//struct request_t {
//    int fd;
//    char * selector;
//    size_t selector_len;
//    char * path;
//    size_t path_len;
//};

#define M_PROCESS 0
#define M_THREAD 1
#define NEED_TO_FREE -2
#define NO_FREE -3
#endif //GOPHER_DEFINITIONS_H

