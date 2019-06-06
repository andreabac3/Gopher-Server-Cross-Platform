//
// Created by andreabacciu on 6/6/19.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "utils.h"

#include "config_file.h"
#include "definitions.h"
#ifdef __unix__
#endif
/*
int utParseConfigurationFile(char *path) {
    FILE *file = fopen(path, "r");
    int CUR_MAX = 4095;
    char *buffer = (char*) malloc(sizeof(char) * CUR_MAX); // allocate buffer.
    char currentline[BASEVALUE];
v
    if (file != NULL) {
        return 1;
    }

    while (fgets(buffer, sizeof(char) * BASEVALUE, file) != NULL) {
        fprintf(stderr, "got line: %s\n", buffer);
    }
    (void) fclose(file);
    return EXIT_SUCCESS;
}



*/

int conf_opts_port_number(char* opt){

    int port = ut_strtoint(opt);
    Assert(port <= 65535 , "The port number is not in the valid range");
    return port;
}


char conf_opts_mode_concurrency(char* opt){

    Assert(strcmp(opt, "Thread") != 0 || strcmp(opt, "Process") != 0, "Unsupported mode_concurrency");

    if (strcmp(opt, "Thread") == 0) return M_THREAD;
    return M_PROCESS;
}


char* conf_opts_root_dir(char* opt){
    return opt;
}


int conf_read_opt(int argc, char *argv[], struct Configs configs){
    int opt;

    while((opt = getopt(argc, argv, "m:d:p:")) != -1) {
        switch(opt) {
            case 'p':
                configs.port_number = conf_opts_port_number(optarg);
                break;
            case 'm':
                configs.mode_concurrency = conf_opts_mode_concurrency(optarg);
                break;
            case 'd':
                configs.root_dir = conf_opts_root_dir(optarg);
                break;
            default:
                help();
                return 1;
        }
    }
    return 0;
}
