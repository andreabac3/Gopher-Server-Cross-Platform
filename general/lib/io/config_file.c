//
// Created by andreabacciu on 6/6/19.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <mach/boolean.h>
#include "utils.h"

#include "config_file.h"
#include "definitions.h"

#ifdef __unix__
#endif


/*
int utParseConfigurationFile(char *path) {
    FILE *file = fopen(path, "r");
    int CUR_MAX = 4095;
    char *buffer = (char*) malloc(sizeof(char) * CUR_MAX); // allocate buffer.
    char currentline[BASEVALUE];
v
    if (file != NULL) {
        return 1;
    }

    while (fgets(buffer, sizeof(char) * BASEVALUE, file) != NULL) {
        fprintf(stderr, "got line: %s\n", buffer);
    }
    (void) fclose(file);
    return EXIT_SUCCESS;
}



*/

int conf_opts_port_number(char *opt) {

    int port = ut_strtoint(opt);
    Assert(port <= 65535, "The port number is not in the valid range");
    return port;
}


char conf_opts_mode_concurrency(char *opt) {

    Assert((strcmp(opt, "Thread") != 0) ^ (strcmp(opt, "Process") != 0), "Unsupported mode_concurrency");

    if (strcmp(opt, "Thread") == 0) return M_THREAD;
    return M_PROCESS;
}


char *conf_opts_root_dir(char *opt) {
    return opt;
}


int conf_read_opt(int argc, char *argv[], struct Configs* configs) {
    int opt;

    while ((opt = getopt(argc, argv, "m:d:p:")) != -1) {
        switch (opt) {
            case 'p':
                configs->port_number = conf_opts_port_number(optarg);
                break;
            case 'm':
                configs->mode_concurrency = conf_opts_mode_concurrency(optarg);
                break;
            case 'd':
                configs->root_dir = conf_opts_root_dir(optarg);
                break;
            default:
                help();
                return 1;
        }
    }
    return 0;
}

int conf_parseConfigFile(char *path, struct Configs *config) {

    char *port = "port";
    char *mod = "mode_concurrency";
    char *rootdir = "root_dir";

    int wrong = 0;

    char **array = NULL;        /* array of pointers to char        */
    char *ln = NULL;            /* NULL forces getline to allocate  */
    size_t n = 0;               /* buf size, 0 use getline default  */
    ssize_t nchr = 0;           /* number of chars actually read    */
    size_t idx = 0;             /* array index for number of lines  */
    size_t it = 0;              /* general iterator variable        */
    size_t lmax = 1024;         /* current array pointer allocation */
    FILE *fp = NULL;            /* file pointer                     */

    if (!(fp = fopen(path, "r"))) { /* open file for reading    */
        fprintf(stderr, "error: file open failed '%s'.", path);
        return 1;
    }

    /* allocate LMAX pointers and set to NULL. Each of the 255 pointers will
       point to (hold the address of) the beginning of each string read from
       the file below. This will allow access to each string with array[x].
    */
    if (!(array = calloc(lmax, sizeof *array))) {
        fprintf(stderr, "error: memory allocation failed.");
        return 1;
    }

    /* prototype - ssize_t getline (char **ln, size_t *n, FILE *fp)
       above we declared: char *ln and size_t n. Why don't they match? Simple,
       we will be passing the address of each to getline, so we simply precede
       the variable with the urinary '&' which forces an addition level of
       dereference making char* char** and size_t size_t *. Now the arguments
       match the prototype.
    */
    while ((nchr = getline(&ln, &n, fp)) != -1)    /* read line    */
    {
        while (nchr > 0 && (ln[nchr - 1] == '\n' || ln[nchr - 1] == '\r'))
            ln[--nchr] = 0;     /* strip newline or carriage rtn    */

        /* allocate & copy ln to array - this will create a block of memory
           to hold each character in ln and copy the characters in ln to that
           memory address. The address will then be stored in array[idx].
           (idx++ just increases idx by 1 so it is ready for the next address)
           There is a lot going on in that simple: array[idx++] = strdup (ln);
        */
        array[idx++] = strdup(ln);

        if (idx == lmax) {      /* if lmax lines reached, realloc   */
            char **tmp = realloc(array, lmax * 2 * sizeof *array);
            if (!tmp)
                return -1;
            array = tmp;
            lmax *= 2;
        }
    }

    if (fp) fclose(fp);        /* close file */
    if (ln) free(ln);          /* free memory allocated to ln  */

    /*
        process/use lines in array as needed
        (simple print all lines example below)
    */

    char *single_word;
    char *saveptr1;

    for (it = 0; it < idx; it++) {
        single_word = strtok_r(array[it], " ", &saveptr1);
        while (single_word != NULL) {

            if (strcmp(port, single_word) == 0) {
                single_word = strtok_r(NULL, " ", &saveptr1);
                wrong |= Assert_nb(single_word != NULL, "Missing port value");
                printf("\n%d\n\n", wrong);
                config->port_number = conf_opts_port_number(single_word); // FUNZIONE VALERIO
                printf("%d", config->port_number);

            } else if (strcmp(mod, single_word) == 0) {
                single_word = strtok_r(NULL, " ", &saveptr1);
                wrong |= Assert_nb(single_word != NULL, "Missing mod value");
                printf("\n%d\n\n", wrong);

                config->mode_concurrency = conf_opts_mode_concurrency(single_word);

            } else if (strcmp(rootdir, single_word) == 0) {
                single_word = strtok_r(NULL, "\"", &saveptr1);
                wrong |= Assert_nb(single_word != NULL, "Missing root dir value");
                config->root_dir = single_word;
            } else {
                for (it = 0; it < idx; it++) {        /* free array memory    */
                    free(array[it]);
                }
                free(array);
                Assert(0, "Unknow parameters in configuration file");
            }
            single_word = NULL;
        }
    }
    printf("\n");
    for (it = 0; it < idx; it++)        /* free array memory    */
        free(array[it]);
    free(array);
    printf("\n%d\n", wrong);
    Assert(wrong == 0, "Something goes wrong in configuration file");
    return 0;

}
