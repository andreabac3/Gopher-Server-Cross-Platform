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

    pid_t d_child1 = fork();
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
    DOS_PROTECTION = 0;
    vlog_ut(1, "%s\n", "Gopher start ...");
    vlog_ut(1, "PID: %d\n", getpid());

    //perror("main#");

#ifdef _WIN32

    // Create named pipe
    //DWORD dwWritten;

    hNamedPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\PipeHandleRequest"),
                            PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
                            1,
                            1024 * 16,
                            1024 * 16,
                            NMPWAIT_USE_DEFAULT_WAIT,
                            NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE){
        windows_perror();
        exit (-125);
    }
    PROCESS_INFORMATION pi;

/*
    ZeroMemory(&pi, sizeof(pi));
    pipe_run_process(&pi);

*/
#endif

    struct Configs c;
    c.reset_config = NULL;
    configs = &c;

    log_ut("Inizio del main\n");



    conf_parseConfigFile(CONFIGURATION_PATH, configs);
    if (conf_read_opt(argc, argv, configs) != 0) {
        return 1;
    }


    vlog_ut(2, "port:%d mode:%d %lu dir:%s\n", configs->port_number, configs->mode_concurrency,
           strlen(configs->root_dir),
           configs->root_dir);



    ut_get_cwd();


#if defined(__unix__) || defined(__APPLE__)

    //getServerIP();
    vlog_ut(1, "\n%s IP DEL SERVER\n", ip_buffer);

    if(!LOG_WITH_MULTIPLE_PROCESS){
        socket_pipe_single_process(global_fd_pipe);
    }
    // return 0;

    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR || signal(SIGCHLD, SIG_IGN)) {
        perror("Signal");
    }

    start_mutex();

    while(true){

        log_ut("conf root dir %s\n", configs->root_dir);
        linux_socket(configs);

        //c.reset_config = NULL;
        //configs = &c;
        configs->reset_config = NULL;
        //free(configs->root_dir);
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);
    }
    log_ut("close server");
    close_mutex();

    if(!LOG_WITH_MULTIPLE_PROCESS){
        close(global_fd_sync_pipe[PIPE_READ]);
    }

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
        printf("%s\n", "sono nel while");
        /*if (configs->mode_concurrency == M_THREAD) {
            ZeroMemory(&pi, sizeof(pi));
            pipe_run_process(&pi, configs->mode_concurrency);
        }
         */
        windows_socket_runner(configs);
        //c.reset_config = NULL;
        //configs = &c;
        configs->reset_config = NULL;
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);
        printf("After EVENT port:%d mode:%d %zu dir:%s\n", configs->port_number, configs->mode_concurrency, strlen(configs->root_dir),
               configs->root_dir);

        /*
        if (mod != configs->mode_concurrency && mod == M_THREAD){
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }*/
    }

    // WaitForSingleObject(pi.hProcess, INFINITE);
    if (configs->mode_concurrency == M_THREAD && 0) {

        CloseHandle(pipe_read);
        CloseHandle(hNamedPipe);


        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
#endif

    /*if (configs->used_OPTARG == false) {
        free(configs->root_dir);
    }*/
    return 0;
    exit(0);
}

