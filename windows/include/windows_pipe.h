//
// Created by andrea on 26-Jul-19.
//

#ifndef GOPHERWIN_WINDOWS_PIPE_H
#define GOPHERWIN_WINDOWS_PIPE_H
#include "definitions.h"
int pipe_simple_write_to_pipe(struct PipeArgs *args);
int pipe_write_to_pipe(char *name, struct PipeArgs *args);
int pipe_run_process(PROCESS_INFORMATION * pi);
#endif //GOPHERWIN_WINDOWS_PIPE_H
