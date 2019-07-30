#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>


#include "definitions.h"
#include "protocol.h"
#include "utils.h"
#include "config_file.h"
#include "files_interaction.h"


#ifdef _WIN32


#include <windows.h>
#include <windows_utils.h>
#include <windows_pipe.h>
#include "windows_socket.h"
#include "windows_events.h"

#include "socket.h"

#endif

#if defined(__unix__) || defined(__APPLE__)

#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux_pipe.h>
#include "linux_files_interaction.h"
#include "files_interaction.h"
#include "linux_thread.h"
#include "linux_socket.h"
#include "linux_signals.h"



// reformat Sh + ò
// comment Sh + ù
// run Sh + Enter
// Ctrl + Enter

void run_in_daemon() {

    __pid_t d_child1 = fork();
    perror("deamon forked");

    if (d_child1 < 0) {
        perror("failed child");
        exit(-1);
    } else if (d_child1 > 1) {
        // father
        perror("creates d_child1");
        exit(0);
    }

    __pid_t d_child2 = fork();
    setsid();
    signal(SIGHUP, SIG_IGN);
    // second child
    if (d_child2 < 0) {
        perror("failed child");

        exit(-1);
    } else if (d_child2 > 1) {
        // father
        perror("creates d_child2");

        exit(0);
    }

}
#endif

int main(int argc, char *argv[]) {

    printf("%s\n", "Gopher start ...");
    printf("PID: %d", getpid());

    //perror("main#");

#ifdef _WIN32


#endif

    struct Configs c;
    c.reset_config = NULL;
    configs = &c;

    printf("Inizio del main\n");


    if (argc == 2) {
        printf("Argomento 1 %s\n", argv[1]);
        printf("Nuovo Processo\n");

        HANDLE hFile = CreateFile(
                "C:\\Users\\Valerio\\CLionProjects\\gopher-project\\fileditext.txt",                // name of the write
                GENERIC_READ | GENERIC_WRITE,          // open for writing
                0,                      // do not share
                NULL,                   // default security
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);                  // no attr. template

        perror("CreateFileMaim");
        exit(0);
    }

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    pipe_run_process(&pi);


    conf_parseConfigFile(CONFIGURATION_PATH, configs);
    printf("\n sono conf.rootdir %s\n", configs->root_dir);
    if (conf_read_opt(argc, argv, configs) != 0) {
        return 1;
    }

    printf("port:%d mode:%d %lu dir:%s\n", configs->port_number, configs->mode_concurrency, strlen(configs->root_dir),
           configs->root_dir);


#if defined(__unix__) || defined(__APPLE__)

    socket_pipe_process();


    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR || signal(SIGCHLD, SIG_IGN)) {
        perror("Signal");
    }

    start_mutex();

    while (true) {

        printf("conf rott dir %s\n", configs->root_dir);
        linux_socket(configs);

        c.reset_config = NULL;
        configs = &c;
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);

    }
    printf("close server");
    close_mutex();

    if (M_THREAD == configs->mode_concurrency) {
        sleep(1);
        pthread_exit(NULL);
    }

#endif
#ifdef _WIN32


    // BOOL running = TRUE;
    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }

    while (true) {
        windows_socket_runner(configs);
        c.reset_config = NULL;
        configs = &c;
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);

    }
    // todo ??
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pipe_read);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

#endif

    if (configs->used_OPTARG == false) {
        free(configs->root_dir);
    }

    exit(0);
}
