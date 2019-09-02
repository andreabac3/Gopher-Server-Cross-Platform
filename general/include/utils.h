#ifndef GOPHER_UTILS_H
#define GOPHER_UTILS_H

#define ASS_CRASH 1
#define ASS_SUCCESS 0

#include <stddef.h>
#include "definitions.h"

struct dict_entry {
    char *str;
    char n;
};
void ut_get_cwd();

int compare(const void *s1, const void *s2);

int fd_is_valid(int fd);

void help();

int ut_strtoint(char *str);

int Assert(int cond, char *message);

int Assert_nb(int cond, char *message);

char *concat(const char *s1, const char *s2);

char *ut_strtok(char *str, const char *delimiters, char **context);

int ut_get_line(char *buf, size_t size);

void ut_clone_configs(struct Configs *c, struct Configs *n);

int log_ut(/*int priority, */const char *format, ...);

int vlog_ut(int verbose, const char *format, ...);


#endif //GOPHER_UTILS_H
