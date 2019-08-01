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

    condition_child = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                              des_cond_child, 0);

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
            printf("RISETTO TUTTE LE CONDIZIONI DA CAPO\n");

            pthread_mutex_lock(mutex_child);
            printf("pthread_mutex_lock\n");

            pthread_cond_wait(condition_child, mutex_child);
            printf("pthread_cond_wait \n");


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

                exit(-1);
            }
            //int n;
            fprintf(stderr, "%s\n", "sono debug3");
            printf("BOOOOOOL : %d", fd_is_valid(fd_pipe[0]));

            char message[BUFFER_SIZE * 2] = {0};
            ssize_t nread = read(fd_pipe[0], message, BUFFER_SIZE * 2);
            // ssize_t nread = read(fd_pipe[0], &data, sizeof(data));
            // printf("%zu", nread);
            fprintf(stderr, "%s %zu\n", "read riuscito -> sono debug", nread);


            printf("---- child process read\n");

            dprintf(fd_log, "Byte %s", message);
            //dprintf(fd_log, "<%s>\n", "bho");

            //int err = fprintf(fp_filelog, "FileName: %s\t%d Byte \t IP Client: %s\n", data->path, data->dim_file, data->ip_client);
            perror("dprintf");
            //write(fd_log, "cia", sizeof("cia"));

            //printf("SONO N %d \n", n);
            pthread_mutex_unlock(mutex_child);

            printf("---- child process close\n");

        }
        pthread_condattr_destroy(&condAttr);
        pthread_mutexattr_destroy(&mutexAttr);
        pthread_mutex_destroy(mutex_child);
        pthread_cond_destroy(condition_child);
        shm_unlink(OKTOWRITE);
        shm_unlink(MESSAGE);
        shm_unlink(MUTEX);
        close(fd_pipe[0]);
        exit(0);
    }
    close(fd_pipe[0]);


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
