//
// Created by Valerio Neri on 2019-06-07.
//

#ifndef GOPHER_LINUX_FILES_INTERACTION_H
#define GOPHER_LINUX_FILES_INTERACTION_H
#endif //GOPHER_LINUX_FILES_INTERACTION_H

#include "definitions.h"
int file_exist (char *filename);
char getGopherCode(char* path);
//int check_type (char* arg, struct dict_entry dict[]);
//int compare(const void *s1, const void *s2);
int linux_memory_mapping(int fd_client, char* filename);

