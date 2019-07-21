//
// Created by valerioneri on 7/10/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


/*Gopher Code for Windows*/
char check_code(char *arg) {

    /* sorted according to str */
    struct dict_entry dict[] = {
            {".bin",                 '9'},
            {".gif",                 'g'},
            {".jpg",                 'I'},
            //{"jpeg",                'I'}, Per qualche motivo non funziona
            {".png",                 'I'},
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
/*char getGopherCode(char *p) {

    char *path= strdup(p);
    char *saveptr;
    char *name, *ext;

//    printf("%s %s %lu %lu", path, str, strlen(path), strlen(str) );

    name = ut_strtok(path, ".", &saveptr);
    ext = ut_strtok(NULL, ".", &saveptr);

//    printf("token: %s \n", name);
//    printf("token: %s \n", ext);

    // Bisogna fare la free dopo strdup
    //free(path);

    printf("strtok: %s %s\n", name, ext);

    if (ext == NULL) {
        printf("strtok directory: %s %s\n", name, ext);
        return '1';
    }
    return check_code(ext);

}*/
char getGopherCode(char *p) {

    char* ext;
    //replace_char(p, '\\', ' ');
    ext = strrchr(p, '.');

    // Todo strrchr non funziona per vie degli \ invece che i /


    //ext = PathFindExtensionA(p);
    printf("getGopherCode %s of %s\n", ext, p);
    if(ext == NULL){
        return -1;
    }
    return check_code(ext);

}
/*End Gopher Code for Windows*/
