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

#endif

#ifdef _WIN32


#include <windows.h>
#include <windows_utils.h>
#include "windows_socket.h"
#include "windows_events.h"

#include "socket.h"
#endif
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

int main(int argc, char *argv[]) {

    printf("%s\n", "Gopher start ...");
    printf("PID: %d", getpid());

    //perror("main#");

#ifdef _WIN32

    printf("SONO ARGC%d\n", argc);

    SECURITY_ATTRIBUTES securityAttributes = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

    if (false == CreatePipe(&pipe_read, &pipe_write, &securityAttributes, BUFFER_SIZE * 2)) {
        windows_perror();
        exit(22);
    }
    char child_cmd[32];
    sprintf(child_cmd, "%lld", (ULONG64) (ULONG_PTR) pipe_read);
    printf(" SONO CHILD CMD %s\n", child_cmd);
    // pipe std out link


    HANDLE father_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (FALSE == SetStdHandle(STD_OUTPUT_HANDLE, pipe_read)) {
        windows_perror();
        printf("FALLITA CREATE SetStdHandle");
        exit(23);

    }


    // process
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    // HANDLE dup_pipe_read;
    if (true == CreateProcess(
            "C:\\Users\\andrea\\CLionProjects\\gopher5\\gopher-project\\cmake-build-debug\\gopherWinSubProcess.exe",
            (char *) child_cmd, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {

        //WaitForSingleObject(pi.hProcess, INFINITE);


        HANDLE dup_pipe_read = NULL;

        if (false ==
            DuplicateHandle(GetCurrentProcess(), pipe_read, pi.hProcess, &dup_pipe_read, DUPLICATE_SAME_ACCESS, FALSE,
                            DUPLICATE_SAME_ACCESS)) {
            windows_perror();
            printf("FALLITA CREATE PIPE");

            exit(23);
        }
        perror("PIPE RIUSCITA CON SUCCESSO\n");
        printf("%d\n", dup_pipe_read);
        printf("%d\n", pipe_read);


    } else {
        perror("create process is failed");
        windows_perror();
        exit(24);
    }

    if (FALSE == SetStdHandle(STD_OUTPUT_HANDLE, father_std_out)) {
        //windows_perror();
        fprintf(stderr,"%s\n","FALLITA CREATE SetStdHandle close");
        exit(23);
    }


#endif

    struct Configs c;
    c.reset_config = NULL;
    configs = &c;


    conf_parseConfigFile(CONFIGURATION_PATH, configs);
    printf("\n sono conf.rootdir %s\n", configs->root_dir);
    if (conf_read_opt(argc, argv, configs) != 0){
        return 1;
    }

    printf("port:%d mode:%d %lu dir:%s\n", configs->port_number, configs->mode_concurrency, strlen(configs->root_dir), configs->root_dir);


#if defined(__unix__) || defined(__APPLE__)

    socket_pipe_process();


    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR || signal(SIGCHLD, SIG_IGN)) {
        perror("Signal");
    }

    start_mutex();

    while (true) {

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
