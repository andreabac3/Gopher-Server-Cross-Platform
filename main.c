#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#include <unistd.h>

#include "definitions.h"
#include "utils.h"
#include "config_file.h"
#include "files_interaction.h"

#ifdef __unix__
#include "linux_files_interaction.h"
#include "linux_thread.h"
#endif


#ifdef _WIN32

#include "windows_socket.h"
#endif
// reformat Sh + ò
// comment Sh + ù
// run Sh + Enter
// Ctrl + Enter

void* thr_test_func(void* str){
    printf("%s %s\n", "Thread print", (char *) str);
    return 0;
}

int main(int argc, char *argv[]) {

    struct Configs configs;
    // configs.root_dir = malloc(50 * sizeof(char));
    // chiamata alla lettura del file di configurazione

    //
    //
    //
    // int windows_socket()conf_parseConfigFile("../gopher_server_configuration.txt", &configs);

    if(conf_read_opt(argc, argv, &configs) != 0) return 1;

    printf("port:%d mode:%d %lu dir:%s\n", configs.port_number, configs.mode_concurrency, strlen(configs.root_dir), configs.root_dir);

#ifdef __unix__
    pthread_t t_id;

    thr_pthread_create(&t_id, &thr_test_func, (void *) "ciao");
    sleep(2);
#endif
#ifdef _WIN32
   windows_socket();
#endif
    return 0;
}
