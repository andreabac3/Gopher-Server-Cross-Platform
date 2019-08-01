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

    // Create named pipe
    DWORD dwWritten;


    hNamedPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\PipeHandleRequest"),
                            PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
                            1,
                            1024 * 16,
                            1024 * 16,
                            NMPWAIT_USE_DEFAULT_WAIT,
                            NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE){
        exit (-125);
    }

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    pipe_run_process(&pi);


#endif

    struct Configs c;
    c.reset_config = NULL;
    configs = &c;

    printf("Inizio del main\n");




    conf_parseConfigFile(CONFIGURATION_PATH, configs);
    if (conf_read_opt(argc, argv, configs) != 0) {
        return 1;
    }

    printf("port:%d mode:%d %lu dir:%s\n", configs->port_number, configs->mode_concurrency, strlen(configs->root_dir),
           configs->root_dir);


#if defined(__unix__) || defined(__APPLE__)

    socket_pipe_new_process2();
    //return 0;

    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR || signal(SIGCHLD, SIG_IGN)) {
        perror("Signal");
    }

    start_mutex();

    while (true) {

        printf("conf root dir %s\n", configs->root_dir);
        linux_socket(configs);

        //c.reset_config = NULL;
        //configs = &c;
        configs->reset_config = NULL;
        //free(configs->root_dir);
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
    CloseHandle(hNamedPipe);


    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

#endif

    /*if (configs->used_OPTARG == false) {
        free(configs->root_dir);
    }*/

    exit(0);
}
