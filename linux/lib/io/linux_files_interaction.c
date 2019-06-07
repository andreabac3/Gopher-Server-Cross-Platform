//
// Created by Valerio Neri on 2019-06-07.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "definitions.h"
#include "utils.h"
#include "linux_files_interaction.h"

char getGopherCode(char* path){
    char* type;
    char* subtype;
    char* strtok_ptr;

    char* command = concat("file -b --mime-type ", path);
    FILE *fp = popen(command, "r");

    if(!fp){
        fprintf(stderr, "Could not open pipe for output.\n");
        return -1;
    }

    char *mime_type = NULL;
    size_t len = 0;

    if (getline(&mime_type, &len, fp) != -1){
        fputs(mime_type, stdout);
    }

    type = strtok_r(mime_type, "/", &strtok_ptr);
    subtype = strtok_r(NULL, "\n", &strtok_ptr);

    printf("Code: <%s %s>\n", type, subtype);

//    struct_test("kill", dict);


    free(mime_type); // ?
    free(command);
    pclose(fp);

    return 0;
}

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

