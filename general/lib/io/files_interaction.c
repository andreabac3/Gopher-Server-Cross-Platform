//
// Created by Valerio Neri on 2019-06-07.
//

#include <stdio.h>
#include "utils.h"
#include "files_interaction.h"

int files_append(char  *string, char *path) {
    FILE *fp;

    fp = fopen(path, "a");
    if (Assert_nb(fp != NULL, "Error opening file.") ){
        return 1;
    }

    fprintf(fp, string);

    fclose(fp);
    return 0;
}




#if __linux__
#include "linux_files_interaction.h"
#endif
#include "linux_files_interaction.h"


int mmap(char* filename, int mode){
    mappLinux(filename, mode);
}

