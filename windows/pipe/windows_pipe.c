
#include <stdio.h>
#include <afxres.h>
#include <windows_utils.h>
#include "windows_pipe.h"

/*
void WriteToPipe(struct PipeArgs pipeArgs1)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data.
{
    DWORD dwRead, dwWritten;
    BOOL bSuccess = FALSE;

    for (;;)
    {
        bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
        if ( ! bSuccess || dwRead == 0 ) break;

        bSuccess = WriteFile(pipeArgs1, pipeArgs1, dwRead, &dwWritten, NULL);
        if ( ! bSuccess ) break;
    }

// Close the pipe handle so the child process stops reading.

    if ( ! CloseHandle(g_hChildStd_IN_Wr) )
        ErrorExit(TEXT("StdInWr CloseHandle"));
}


void ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT.
// Stop when there is no more data.
{
    DWORD dwRead, dwWritten;
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    for (;;)
    {
        bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if( ! bSuccess || dwRead == 0 ) break;

        bSuccess = WriteFile(hParentStdOut, chBuf,
                             dwRead, &dwWritten, NULL);
        if (! bSuccess ) break;
    }
}

*/

int pipe_write_to_pipe(char *name, struct PipeArgs *args) {
    HANDLE hPipe;
    DWORD dwWritten;
    hPipe = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("Error: %d", GetLastError());
        windows_perror();
    }
    char message[BUFFER_SIZE*2] = {0};
    int n = sprintf(message, "W -> FileName: %s\t%d KByte \t IP Client: %s\n", args->path, args->dim_file, args->ip_client);

    if (hPipe != INVALID_HANDLE_VALUE) {
        DWORD numWritten;

        WriteFile(hPipe, message, BUFFER_SIZE*2, &numWritten, NULL);
        //WriteFile(hPipe, args, sizeof(*args), &dwWritten, // number of written bytes NULL);

        CloseHandle(hPipe);
    } else {
        windows_perror();
    }

    printf("%lu\n", dwWritten);
    printf("%zu\n", sizeof(*args));

    return 0;
}

