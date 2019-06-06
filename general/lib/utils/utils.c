//
// Created by valerioneri on 6/5/19.
//

#include <stdio.h>
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#include "config_file.h"

#ifdef __unix__

#include "gopher_server_configuration_linux.h"

#endif

void help(){
    char* helpString = "";
    printf("%s\n", helpString);
}

// Note that free is needed.
char* concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int ut_strtoint(char* str){
    char* end;
    int i = (int) strtol(str, &end, 10);

    Assert(strcmp(end, "") == 0 , "ut_strtoint, The value is not a valid integer");
    return i;
}

int Assert(int cond, char* message){
    if(!cond){
        fprintf(stderr, "%s", message);
        exit(3);
    }
    return cond;
}



