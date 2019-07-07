//
// Created by Valerio Neri on 2019-06-07.
//

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#include "utils.h"
#include "files_interaction.h"

int files_append(char  *string, char *path) {
    FILE *fp;

    fp = fopen(path, "a");
    if (Assert_nb(fp != NULL, "Error opening file.") ){
        return 1;
    }

    fprintf(fp, "%s", string);

    fclose(fp);
    return 0;
}


int protocolResponse(int type, char *filename, char *path, const char *host, int port, char *result){
    int ret = Assert_nb(0 < type && type < 10, "protocolResponse -> Error invalid type.");
    ret |= Assert_nb( 0 < port && port < 65536  , "protocolResponse2 -> Error invalid port");
    if (ret > 0){
        return ret;
    }
    if (type == 3){ // ERROR type 3
        // format
        // 3 '/gopher/clients/sdas.txt' doesn't exist!		error.host	1
        sprintf(result,"%s %s%s %s %d", "3 ", path, filename , " doesn't exist! error.host", port);
    }else {
        // format
        // type + filename + host + port
        sprintf(result, "%d %s %s %s %d", type, filename, path, host, port);
    }
    return 0;
}

#if __linux__
#include "linux_files_interaction.h"
int files_memory_map(char* filename, int mode){
    return mappLinux(filename, mode);
}
#endif




