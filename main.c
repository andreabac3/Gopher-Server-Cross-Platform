#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <protocol.h>


#include "definitions.h"
#include "utils.h"
#include "config_file.h"
#include "files_interaction.h"

#if defined(__unix__) || defined(__APPLE__)
#include <signal.h>
#include "linux_files_interaction.h"
#include "files_interaction.h"
#include "linux_thread.h"
#include "linux_socket.h"
#include "linux_signals.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include "windows_socket.h"
#endif
// reformat Sh + ò
// comment Sh + ù
// run Sh + Enter
// Ctrl + Enter

int main(int argc, char *argv[]) {

    struct Configs c;
    c.reset_config = NULL ;
    configs = &c;


    // configs.root_dir = malloc(50 * sizeof(char));
    // chiamata alla lettura del file di configurazione
    //
    printf("PID: %ld  PPID: %ld\n", (long)getpid());

    //
    conf_parseConfigFile(CONFIGURATION_PATH, configs);
    printf("\n sono conf.rootdir %s\n", configs->root_dir);
    if (conf_read_opt(argc, argv, configs) != 0) return 1;
    /*
    configs.port_number = 7070;
    configs.mode_concurrency=1;
    configs.root_dir="/sda";
    */
    printf("port:%d mode:%d %lu dir:%s\n", configs->port_number, configs->mode_concurrency, strlen(configs->root_dir),
           configs->root_dir);


#if defined(__unix__) || defined(__APPLE__)

    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR)

    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR)

    printf("%c \n", getGopherCode("C:/Users/valerio/file.png"));
    while(true) {
        linux_socket(configs);
        c.reset_config = NULL ;
        configs = &c;
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);

    }
    //pthread_t t_id;

    //thr_pthread_create(&t_id, &thr_test_func, (void *) "lol");
    //sleep(2);

#endif
#ifdef _WIN32
    windows_socket_runner(configs);

#endif
    //printf("%s", configs.root_dir);
    if (configs->useOPTARG == false) {
        free(configs->root_dir);
    }
    //sleep(2);
    //return 0;
#if defined(__unix__) || defined(__APPLE__)

    if (M_THREAD == configs->mode_concurrency) {
        sleep(1);
        pthread_exit(NULL);
    }
#endif

    exit(0);
    //pthread_exit(&ret);
}
