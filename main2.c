//
// Created by andrea on 26-Jul-19.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <definitions.h>
#include <windows_utils.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    printf("SONO PARTITO");
    FILE *fp_prova = fopen(LOG_PATH, "a+");
    //if (argc == 1) {

    //    if (0 == strcmp(argv[0], "readPipe")) {


    printf("Argomento 1 %s\n", argv[0]);
    struct PipeArgs data;
    DWORD dwRead;
    printf("%s\n", "Nuovo Processo");

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


