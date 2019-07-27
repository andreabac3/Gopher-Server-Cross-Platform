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
#include <sys/mman.h>


#include <stddef.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>


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
    __pid_t d_child2 = fork();
    perror("deamon forked");

    if (d_child1 < 0) {
        perror("failed child");
        exit(-1);
    } else if (d_child1 > 1) {
        // father
        perror("creates d_child1");
        exit(0);
    }

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

    run_in_daemon();

    // main code
    if (pipe(fd_pipe) < 0) {
        perror("pipe");
        exit(-1);

    }
    child = fork();
    if (child < 0) {
        perror("failed child");
        exit(-1);
    } else if (child == 0) {
        printf("FIGLIO È PARTITO IN ATTESA SULLA COND VARIABLE\n");
        close(fd_pipe[1]);

        while (true) {
            fprintf(stderr, "%s\n", "sono debug3");
            printf("RISETTO TUTTE LE CONDIZIONI DA CAPO\n");


            pthread_cond_t *condition_child;
            pthread_mutex_t *mutex_child;
            int des_cond_child, des_msg, des_mutex_child;


            des_mutex_child = shm_open(MUTEX, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

            if (des_mutex_child < 0) {
                perror("failure on shm_open on des_mutex");
                exit(1);

            }

            if (ftruncate(des_mutex_child, sizeof(pthread_mutex_t)) == -1) {
                perror("Error on ftruncate to sizeof pthread_cond_t\n");
                exit(-1);
            }

            mutex_child = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
                                                   PROT_READ | PROT_WRITE, MAP_SHARED, des_mutex_child, 0);

            if (mutex_child == MAP_FAILED) {
                perror("Error on mmap on mutex\n");
                munmap(mutex_child, sizeof(pthread_mutex_t));
                exit(1);
            }

            des_cond_child = shm_open(OKTOWRITE, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

            if (des_cond_child < 0) {
                perror("failure on shm_open on des_cond");
                exit(1);
            }

            if (ftruncate(des_cond_child, sizeof(pthread_cond_t)) == -1) {
                perror("Error on ftruncate to sizeof pthread_cond_t\n");
                exit(-1);
            }

            condition_child = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t),
                                                      PROT_READ | PROT_WRITE, MAP_SHARED, des_cond_child, 0);

            if (condition_child == MAP_FAILED) {
                perror("Error on mmap on condition\n");
                munmap(mutex_child, sizeof(pthread_mutex_t));
                munmap(condition_child, sizeof(pthread_cond_t));

                exit(1);
            }
            pthread_mutexattr_t mutexAttr;
            pthread_mutexattr_init(&mutexAttr);
            pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(mutex_child, &mutexAttr);

            /* set condition shared between processes */
            pthread_condattr_t condAttr;
            pthread_condattr_init(&condAttr);
            pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
            pthread_cond_init(condition_child, &condAttr);

            printf("CHILD waits on condition\n");

            pthread_mutex_lock(mutex_child);
            printf("CHILD waits on condition2\n");

            pthread_cond_wait(condition_child, mutex_child);
            printf("CHILD waits on condition3\n");

            pthread_mutex_unlock(mutex_child);

            printf("Signaled by PARENT process, wake up!!!!!!!!\n");

            // TODO INSERT CONDITION VARIABLE and while true
            printf("---- child process wrote\n");
            //FILE* fp_fileLog = fopen(LOG_PATH, "w");

            int fd_log = open(LOG_PATH, O_WRONLY | O_APPEND);
            //FILE* fp_filelog= fdopen(fd_log, "a");
            printf("---- child process open\n");
            if (fd_log == -1) {
                //if (fp_fileLog == NULL){
                printf("sono bloccato");
                pthread_condattr_destroy(&condAttr);
                pthread_mutexattr_destroy(&mutexAttr);
                pthread_mutex_destroy(mutex_child);
                pthread_cond_destroy(condition_child);

                shm_unlink(OKTOWRITE);
                shm_unlink(MESSAGE);
                shm_unlink(MUTEX);
                exit(-1);
            }
            //int n;
            struct PipeArgs data;
            fprintf(stderr, "%s\n", "sono debug3");
            printf("BOOOOOOL : %d", fd_is_valid(fd_pipe[0]));

            char message[BUFFER_SIZE*2] = {0};
            ssize_t nread = read(fd_pipe[0], message, BUFFER_SIZE*2);
            // ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
            // printf("%zu", nread);
            fprintf(stderr, "%s %zu\n", "read riuscito -> sono debug", nread);


            printf("---- child process read\n");


            ;
            /*
            printf("FileName: %s\n", data.path);
            printf("%d Byte \n", data.dim_file);
            printf("IP Client: %s\n", data.ip_client);



             dprintf(fd_log, "FileName: %s\t%d Byte \t IP Client: %s\n", data.path, data.dim_file, data.ip_client);
            */

            dprintf(fd_log, "Byte %s", message);
            //dprintf(fd_log, "<%s>\n", "bho");

            //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
            perror("dprintf");
            //write(fd_log, "cia", sizeof("cia"));

            //printf("SONO N %d \n", n);

            pthread_condattr_destroy(&condAttr);
            pthread_mutexattr_destroy(&mutexAttr);
            pthread_mutex_destroy(mutex_child);
            pthread_cond_destroy(condition_child);

            shm_unlink(OKTOWRITE);
            shm_unlink(MESSAGE);
            shm_unlink(MUTEX);
            printf("---- child process close\n");


        }
        close(fd_pipe[0]);
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
