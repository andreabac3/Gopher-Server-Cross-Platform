//
// Created by Valerio Neri on 2019-06-06.
//
//#pragma once
#ifndef GOPHER_DEFINITIONS_H
#define GOPHER_DEFINITIONS_H

struct Configs {
    unsigned int  port_number;
    char  mode_concurrency;
    char*  root_dir;
};

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
    char * display;
    char * selector;
    char * host;
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

#endif //GOPHER_DEFINITIONS_H

