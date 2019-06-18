//
// Created by valerioneri on 6/5/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "definitions.h"


#if defined(__unix__) || defined(__APPLE__)

//#include "gopher_server_configuration_linux.h"

#endif

void help() {
    char *helpString = "";
    printf("%s\n", helpString);
}

/*
 * CONCAT
 * Note that free is needed.
 *
 */
char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

int ut_strtoint(char *str) {
    char *end;
    int i = (int) strtol(str, &end, 10);

    Assert(strcmp(end, "") == 0, "ut_strtoint, The value is not a valid integer");
    return i;
}

char* ut_strtok(char* str, const char* delimiters, char** context){
#if defined(__unix__) || defined(__APPLE__)
    return strtok_r(str, delimiters, context);
#endif

#ifdef _WIN32
    return "ciao";
    //return strtok_s(str, delimiters, context);
#endif
}

/*
 * Assert return 0 if success, no error.
 */
int Assert(int cond, char *message) {
    if (!cond) {
        fprintf(stderr, "%s\n", message);
        exit(3);
    }
    return !cond;
}

/*
 * Assert_nb return 0 if success, no error.
 */
int Assert_nb(int cond, char *message) {
    if (!cond) {
        fprintf(stderr, "%s\n", message);
    }
    return !cond;
}


