#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <definitions.h>
#include <windows_utils.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    HANDLE eventReadyToReadPipe = CreateEventA(NULL, FALSE, TRUE, "eventReadyToReadPipe");
    do {
        WaitForSingleObject(eventReadyToReadPipe, INFINITE);
        HANDLE child_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
        char buf[BUFFER_SIZE * 2] = {0};
        DWORD numRead;

        if (ReadFile(child_std_out, buf, BUFFER_SIZE * 2, &numRead, NULL)) {
            fprintf(stderr, "NumRead PIPE%d \n%s %s\n", numRead, buf, argv[1]);
            FILE *fp_log = fopen(LOG_PATH, "a+");
            fprintf(fp_log, "%s", buf);
            fclose(fp_log);
        }
    } while (atoi(argv[1]) == M_THREAD && 0);
    CloseHandle(eventReadyToReadPipe);


    exit(0);
}



