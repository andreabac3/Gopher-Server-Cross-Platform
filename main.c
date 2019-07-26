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
#include <fcntl.h>
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


int main(int argc, char *argv[]) {

    printf("%s\n", "Gopher start ...");

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


// configs.root_dir = malloc(50 * sizeof(char));
// chiamata alla lettura del file di configurazione
//
    printf("PID: %ld  PPID: %ld\n", (long)

            getpid()

    );

//
    conf_parseConfigFile(CONFIGURATION_PATH, configs);
    printf("\n sono conf.rootdir %s\n", configs->root_dir);
    if (
            conf_read_opt(argc, argv, configs
            ) != 0)
        return 1;
/*
configs.port_number = 7070;
configs.mode_concurrency=1;
configs.root_dir="/sda";
*/
    printf("port:%d mode:%d %lu dir:%s\n", configs->port_number, configs->mode_concurrency,
           strlen(configs
                          ->root_dir),
           configs->root_dir);


#if defined(__unix__) || defined(__APPLE__)

    if (pipe(fd_pipe) < 0) {
        perror("pipe");
        exit(-1);

    }
    child = fork();
    if (child < 0) {
        perror("failed child");
        exit(-1);
    } else if (child == 0) {

        // TODO INSERT CONDITION VARIABLE and while true
        close(fd_pipe[1]);
        printf("---- child process wrote\n");
        //FILE* fp_fileLog = fopen(LOG_PATH, "w");
        int fd_log = open(LOG_PATH, O_WRONLY | O_APPEND);
        //FILE* fp_filelog= fdopen(fd_log, "a");
        printf("---- child process open\n");
        if (fd_log == -1) {
            //if (fp_fileLog == NULL){
            printf("sono bloccato");
            exit(-1);
        }
        //int n;
        struct PipeArgs data;

        //ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
        ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
        printf("%zu", nread);


        printf("%zu", nread);

        printf("---- child process read\n");


        //printf("\n sono figlio :-> %s\n", data->ip_client);
        printf("FileName: %s\n", data.path);
        printf("%d Byte \n", data.dim_file);
        printf("IP Client: %s\n", data.ip_client);

        /*int err = */ dprintf(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file,
                               data.ip_client);
        //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
        perror("dprintf");
        //write(fd_log, "cia", sizeof("cia"));

        //printf("SONO N %d \n", n);
        close(fd_pipe[0]);

        printf("---- child process close\n");
        exit(0);
    }


    if (signal(SIGHUP, signal_sighup_handler) == SIG_ERR) {
        perror("Signal");
    }

    printf("%c \n", getGopherCode("C:/Users/valerio/file.png"));

    start_mutex();

    linux_socket(configs);
    while (true) {

        c.reset_config = NULL;
        configs = &c;
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);

        linux_socket(configs);
    }

    close_mutex();

    //pthread_t t_id;

    //thr_pthread_create(&t_id, &thr_test_func, (void *) "lol");
    //sleep(2);

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
//printf("%s", configs.root_dir);
    if (configs->used_OPTARG == false) {
        free(configs
                     ->root_dir);
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
