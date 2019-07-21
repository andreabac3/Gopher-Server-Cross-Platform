#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "utils.h"
#include "files_interaction.h"
#include "ut_dict.h"

int file_exist(char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0); // ritorna diverso da zero se vero
}

int file_type(char *filename) {
    struct stat s;
    if (stat(filename, &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            return 1;
        } else if (s.st_mode & S_IFREG) {
            return 2;
        } else {
            return 3;
        }
    } else {
        return 0;
    }
}

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


#if __linux__
#include "linux_files_interaction.h"
int files_memory_map(char* filename, int mode){
    // TODO Errore di compilazione
    return 0;
    //return mappLinux(filename, mode);
}
#endif




