#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>

#include "definitions.h"
#include "utils.h"
#include "config_file.h"
#include "files_interaction.h"

#if defined(__unix__) || defined(__APPLE__)

#include "linux_files_interaction.h"
#include "linux_thread.h"
#include "linux_socket.h"

#endif

#ifdef _WIN32
#include "windows_socket.h"
#include <windows.h>
#endif
// reformat Sh + ò
// comment Sh + ù
// run Sh + Enter
// Ctrl + Enter


int main(int argc, char *argv[]) {

    struct Configs configs;
    // configs.root_dir = malloc(50 * sizeof(char));
    // chiamata alla lettura del file di configurazione

    //
    //
    //
    conf_parseConfigFile("../gopher_server_configuration.txt", &configs);
    printf("\n sono conf.rootdir %s\n", configs.root_dir);
    if (conf_read_opt(argc, argv, &configs) != 0) return 1;
    /*
    configs.port_number = 7070;
    configs.mode_concurrency=1;
    configs.root_dir="/sda";
    */
    printf("port:%d mode:%d %lu dir:%s\n", configs.port_number, configs.mode_concurrency, strlen(configs.root_dir),
           configs.root_dir);



    char *bufferTESToutputSocket = calloc( 300 , sizeof(  char) );
    // protocolResponse(unsigned int type, char* filename, char* path, const char * host, unsigned int port, char* result );
    int risultato = protocolResponse(1, "prova.txt", "/var/", "localhost", 7070, bufferTESToutputSocket);
    if (0 != risultato){
        free(bufferTESToutputSocket);
        exit(1);
    }
    printf("%s\n", bufferTESToutputSocket);
    free(bufferTESToutputSocket);



#if defined(__unix__) || defined(__APPLE__)

    linux_socket(configs);

    //pthread_t t_id;

    //thr_pthread_create(&t_id, &thr_test_func, (void *) "lol");
    //sleep(2);
#endif
#ifdef _WIN32
    windows_socket(configs);

#endif
    free(configs.root_dir);

    return 0;
}
