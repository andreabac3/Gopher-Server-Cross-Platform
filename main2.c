#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <definitions.h>
#include <windows_utils.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    printf("\nSONO PARTITO\n");

    //printf("%s" , argv[0]);
    HANDLE eventReadyToReadPipe = CreateEventA(NULL, FALSE, TRUE, "eventReadyToReadPipe");
    while (TRUE) {
        WaitForSingleObject(eventReadyToReadPipe, INFINITE);
        HANDLE child_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
        char buf[BUFFER_SIZE * 2] = {0};
        DWORD numRead;

        if(ReadFile(child_std_out, buf, BUFFER_SIZE * 2, &numRead, NULL)){
            fprintf(stderr, "NumRead PIPE%d \n%s\n", numRead ,buf);
            FILE *fp_log = fopen(LOG_PATH, "a+");
            fprintf(fp_log, "> %s\n", buf);
            fclose(fp_log);
        }
    }
    CloseHandle(eventReadyToReadPipe);


    /*_sleep(100);
    fprintf(stderr, "\n%s\n", argv[0]);
    _sleep(100);
    fprintf(stderr, " SONO argv[0] |%d|\n", atoi(argv[0]));
    */



    exit(0);
    /*
    HANDLE hWrite = 0;
    wchar_t* sz;
    if (sz = wcschr(GetCommandLineW(), '<'))
    {
        fprintf(stderr, " SONO SZ %s\n", sz);
        //hWrite = (HANDLE)(ULONG_PTR)_wcstoi64(sz + 1, &sz, 16);
        if (*sz != '>')
        {
            hWrite = 0;
        }
    }

    exit(0);
    FILE *fp_prova = fopen(LOG_PATH, "a+");
    //if (argc == 1) {

    //    if (0 == strcmp(argv[0], "readPipe")) {


    printf("Argomento 1 %s\n", argv[0]);
    struct PipeArgs data;
    DWORD dwRead;
    printf("%s\n", "Nuovo Processo");
    */
    /*
     * HANDLE hPipe = CreateNamedPipe("\\\\.\\pipe\\Pipe",
                                   PIPE_ACCESS_DUPLEX,
                                   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE |
                                   PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
                                   1,
                                   1024 * 16,
                                   1024 * 16,
                                   NMPWAIT_USE_DEFAULT_WAIT,
                                   NULL);
    */
    /*
    DWORD numRead;
    while (true) {
        HANDLE hPipe = CreateNamedPipe(PIPE_LOG_NAME, PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND, PIPE_WAIT, 1, 1024,
                                       1024, 120 * 1000, NULL);
        if (hPipe == INVALID_HANDLE_VALUE) {
            windows_perror();
            printf("Error: %d", GetLastError());
        }

        char str[BUFFER_SIZE * 2] = {0};
        if (ConnectNamedPipe(hPipe, NULL) != false) {
            ReadFile(hPipe, str, BUFFER_SIZE * 2, &numRead, NULL);
            fprintf(fp_prova, "> %s\n", str);
            printf("%s", str);
        }
        CloseHandle(hPipe);
    }
    fclose(fp_prova); //hPipe = INVALID_HANDLE_VALUE;
    return 0;
     */
}

/*
    //windows_perror();
    while (hPipe != INVALID_HANDLE_VALUE) {
        printf("%s", "sono nel while by 2processo");
        fprintf(fp_prova, "%s\n", "ciao2");

        if (ConnectNamedPipe(hPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
        {
            fprintf(fp_prova, "%s\n", "ciao3");

            while (ReadFile(hPipe, &data, sizeof(data) - 1, &dwRead, NULL) != FALSE) {
                //buffer[dwRead] = '\0';
                fprintf(fp_prova, "%s\n", data.ip_client);

                //printf("%s", data.ip_client);
                //printf("%s", buffer);
            }
        }

        DisconnectNamedPipe(hPipe);
    }

    printf("%s", "Nuovo processo sta per essere killato");
    fclose(fp_prova);
    exit(0);
    //    }
    //}
    return 0;
}
*/


