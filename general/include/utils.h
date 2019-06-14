//
// Created by valerioneri on 6/5/19.
//

#ifndef GOPHER_UTILS_H
#define GOPHER_UTILS_H

void help();
#endif //GOPHER_UTILS_H
int ut_strtoint(char* str);
int Assert(int cond, char* message);
int Assert_nb(int cond, char* message);
char* concat(const char *s1, const char *s2);
char* ut_strtok(char* str, const char* delimiters, char** context);
