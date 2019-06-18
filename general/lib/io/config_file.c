//
// Created by andreabacciu on 6/6/19.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include "config_file.h"
#include "definitions.h"
#include "utils.h"


#if defined(__unix__) || defined(__APPLE__)
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


int conf_read_opt(int argc, char *argv[], struct Configs *configs) {
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

#include <stdio.h>  /* defines FILENAME_MAX */
//#define WINDOWS  /* uncomment this line to use it for windows.*/
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
int conf_parseConfigFile(char *path, struct Configs *config) {
    char buff[FILENAME_MAX];
    GetCurrentDir( buff, FILENAME_MAX );
    printf("%s" , buff);
    char *port = "port";
    char *mod = "mode_concurrency";
    char *rootdir = "root_dir";

    unsigned int wrong = 0;

    char **StringsArray = NULL;        /* array of pointers to char        */
    char *ln = NULL;            /* NULL forces getline to allocate  */
    size_t n = 0;               /* buf size, 0 use getline default  */
    ssize_t nchr = 0;           /* number of chars actually read    */
    size_t idx = 0;             /* array index for number of lines  */
    size_t it = 0;              /* general iterator variable        */
    size_t lmax = 1024;         /* current array pointer allocation */
    FILE *fp = NULL;            /* file pointer                     */


    Assert((fp = fopen(path, "r")) != NULL, "error: configuration file open failed");

    /* allocate LMAX pointers and set to NULL. Each of the 255 pointers will
       point to (hold the address of) the beginning of each string read from
       the file below. This will allow access to each string with array[x].
    */
    //printf("size %zu %zu \n", sizeof *StringsArray, sizeof(char **)),
    Assert((StringsArray = calloc(lmax, sizeof *StringsArray)) != NULL, "error: memory allocation failed.");


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
        StringsArray[idx++] = strdup(ln);

        if (idx == lmax) {      /* if lmax lines reached, realloc   */
            char **tmp = realloc(StringsArray, lmax * 2 * sizeof *StringsArray);
            if (!tmp)
                return -1;
            StringsArray = tmp;
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

    // for all lines
    for(int j = 0; j < idx; j++){
        printf(StringsArray[j]);
    }
    for (it = 0; it < idx; it++) {
        // name of the option
        single_word = ut_strtok(StringsArray[it], " ", &saveptr1);
        printf("\n singleword->  %s \n" , single_word);
        // option value evaluation
        if (single_word != NULL) {

            if (strcmp(port, single_word) == 0) {
                // if the option is port
                single_word = ut_strtok(NULL, " ", &saveptr1);
                wrong |= (unsigned) Assert_nb(single_word != NULL, "Missing port value");
                config->port_number = conf_opts_port_number(single_word); // FUNZIONE VALERIO

            } else if (strcmp(mod, single_word) == 0) {

                // if the option is mode_concurrency
                single_word = ut_strtok(NULL, " ", &saveptr1);
                wrong |= (unsigned) Assert_nb(single_word != NULL, "Missing mod value");

                config->mode_concurrency = conf_opts_mode_concurrency(single_word);

            } else if (strcmp(rootdir, single_word) == 0 ) {
                // if the option is root_dir
                single_word = ut_strtok(NULL, "\"", &saveptr1);
                wrong |= (unsigned) Assert_nb(single_word != NULL, "Missing root dir value");
                config->root_dir = calloc(sizeof(char), strlen(single_word)+1);
                strcpy(config->root_dir , single_word);
                printf("%s %s \n", single_word, config->root_dir);
            } else {

                // if the option is unknown (Error)
                for (it = 0; it < idx; it++) {        /* free array memory    */
                    free(StringsArray[it]);
                }
                free(StringsArray);
                Assert(0, "Unknow parameters in configuration file");
            }
        }
    }

    for (it = 0; it < idx; it++)
        free(StringsArray[it]);

    free(StringsArray);
//    printf("\n%d\n", wrong);
    if (Assert_nb(wrong == 0, "Something goes wrong in configuration file") == ASS_CRASH){
        free(config->root_dir);
        exit(1);
    }
    printf("\n prima di fare return  -->  %s\n " , config->root_dir);
    return 0;

}
