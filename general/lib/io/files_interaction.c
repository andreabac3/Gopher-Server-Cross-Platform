//
// Created by Valerio Neri on 2019-06-07.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "files_interaction.h"
#include "ut_dict.h"


int files_append(char *string, char *path) {
    FILE *fp;

    fp = fopen(path, "a");
    if (Assert_nb(fp != NULL, "Error opening file.")) {
        return 1;
    }

    fprintf(fp, "%s", string);

    fclose(fp);
    return 0;
}


char check_code(char *arg) {

    /* sorted according to str */
    struct dict_entry dict[] = {
            {"bin",                 '9'},
            {"gif",                 'g'},
            {"jpg",                 'I'},
            //{"jpeg",                'I'}, Per qualche motivo non funziona
            {"png",                 'I'},
    };

    struct dict_entry *result, key = {arg};

    result = bsearch(&key, dict, sizeof(dict) / sizeof(dict[0]),
                     sizeof dict[0], compare);
    if (!result)
        return '0';

//    printf("Dict Result: %c\n", result->n);
    return result->n;
}


// Gopher Code for windows
char win_getGopherCode(char *p) {

    char *path= strdup(p);
    char *saveptr;
    char *ext;

//    printf("%s %s %lu %lu", path, str, strlen(path), strlen(str) );

    ext = ut_strtok(path, ".", &saveptr);
    ext = ut_strtok(NULL, ".", &saveptr);

//    printf("token: %s \n", name);
//    printf("token: %s \n", ext);

    // Bisogna fare la free dopo strdup
    //free(path);

    if (ext == NULL) {
        return '1';
    }
    return check_code(ext);

}

#if __linux__

#include "linux_files_interaction.h"

int files_memory_map(char *filename, int mode) {
    return mappLinux(filename, mode);
}

#endif




