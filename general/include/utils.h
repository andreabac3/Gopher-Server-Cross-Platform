//
// Created by valerioneri on 6/5/19.
//

#ifndef GOPHER_UTILS_H
#define GOPHER_UTILS_H

#include "definitions.h"

void help();
#endif //GOPHER_UTILS_H
int ut_strtoint(char* str);
int Assert(int cond, char* message);
int Assert_nb(int cond, char* message);
char* concat(const char *s1, const char *s2);

int struct_test (char* arg, struct dict_entry dict[]);
int compare(const void *s1, const void *s2);


