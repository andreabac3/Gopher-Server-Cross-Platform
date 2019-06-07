//
// Created by Valerio Neri on 2019-06-07.
//

#include <stdio.h>
#include "files_interaction.h"

int files_append(char *string, char *path) {
    FILE *fp;

    fp = fopen(path, "a");
    if (fp == NULL) {
        perror("Error opening file.");
    } else {
        fprintf(fp, string);
    }
    fclose(fp);

}
