//#pragma once
#ifndef GOPHER_DEFINITIONS_H
#define GOPHER_DEFINITIONS_H

// START FILES definition of file types
#define FILES_NOT_EXIST 0
#define FILES_IS_DIRECTORY 1
#define FILES_IS_REG_FILE 2
#define FILES_IS_UNMANAGED_FILE 3
#define FILES_NOT_PERMITTED 4

// END FILES definition of file types

#define SOCK_START_TIMEOUT 1
#define SOCK_LOOP_TIMEOUT 2

#define LOG_PATH "../gopher_log_file.txt"
#define CONFIGURATION_PATH "../gopher_server_configuration.txt"
#define CONNECTION_QUEUE 500
#define MAX_CONNECTIONS_ALLOWED 1
#define BUFFER_SIZE 2048

#define SEND_BUFFER_SIZE 512
char cwd[BUFFER_SIZE];
#if defined(__unix__) || defined(__APPLE__)
#define OS_SEPARATOR '/'
#include <fcntl.h>
#include <pthread.h>

#define SOCKET_NON_BLOCKING MSG_DONTWAIT
#endif
#ifdef _WIN32
#define SOCKET_NON_BLOCKING WSAEWOULDBLOCK
#define OS_SEPARATOR '\\'


#define PIPE_LOG_NAME "\\\\.\\pipe\\Pipe"

#endif

struct sendArgs {
    int fd;
    char* buff;
};
struct PipeArgs {
    char *ip_client;
    int dim_file;
    char *path;
    int port;
};

struct Configs {
    unsigned int port_number;
    char mode_concurrency;
    //char *root_dir;
    char root_dir[BUFFER_SIZE];
    int used_OPTARG;
    int *reset_config;
};
struct Configs *configs;
#if defined(__unix__) || defined(__APPLE__)
struct ThreadArgs {
    int fd;
    struct Configs configs;
    char* ip_client;
    int type_Request;
};
#define LOG_WITH_MULTIPLE_PROCESS false

#define PIPE_NUM 2
#define PIPE_WRITE 1
#define PIPE_READ 0
#define MUTEX "/mutex_lock"
#define SYNC_MUTEX "/sync_mutex_lock"

#define OKTOWRITE "/condwrite"
#define MESSAGE "/msg"

pid_t global_child;
int global_fd_pipe[PIPE_NUM];
int global_fd_sync_pipe[PIPE_NUM];

//pthread_cond_t *condition_child;
//pthread_mutex_t *mutex_child;
//int des_cond_child, des_mutex_child;

pthread_cond_t* condition;
pthread_mutex_t* mutex;
pthread_mutex_t* sync_pipe_mutex;
char* message;
int des_cond, des_msg, des_mutex, des_sync_mutex;
//int mode = S_IRWXU | S_IRWXG;

struct MemoryMappingArgs {
    int fd;
    char mode_concurrency;
    char* path;
};

struct SendFileArgs {
    int fd_client;
    char *message_to_send;
    int message_len;
};

#endif
#ifdef _WIN32

#include <afxres.h>
SOCKET server;
struct ThreadArgs {
    SOCKET fd;
    struct Configs configs;
    char *ip_client;
    int type_Request;
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
char ip_buffer[BUFFER_SIZE];

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

