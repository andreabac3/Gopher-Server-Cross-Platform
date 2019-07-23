#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "definitions.h"


#if defined(__unix__) || defined(__APPLE__)

//#include "gopher_server_configuration_linux.h"

#endif
#ifdef __cplusplus
#include <string>

#include <iostream>

using namespace std;
#endif
#include <string.h>
void help() {
    char *helpString = "";
    printf("%s\n", helpString);
}

/*
 * CONCAT
 * Note that free is needed.
 *
 */
char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

int ut_strtoint(char *str) {
    char *end;
    int i = (int) strtol(str, &end, 10);

    Assert(strcmp(end, "") == 0, "ut_strtoint, The value is not a valid integer");
    return i;
}
char *ut_win_strtok_r(char *str, const char *delim, char **save)
{
    char *res, *last;

    if( !save )
        return strtok(str, delim);
    if( !str && !(str = *save) )
        return NULL;
    last = str + strlen(str);
    if( (*save = res = strtok(str, delim)) )
    {
        *save += strlen(res);
        if( *save < last )
            (*save)++;
        else
            *save = NULL;
    }
    return res;
}
char* ut_strtok(char* str, const char* delimiters, char** context){
#if defined(__unix__) || defined(__APPLE__)

    return strtok_r(str, delimiters, context);
#endif

#ifdef _WIN32
    //return "ciao";
    return (char *) ut_win_strtok_r(str, delimiters, context);
#endif
}

/*
 * Assert return 0 if success, no error.
 */
int Assert(int cond, char *message) {
    if (!cond) {
        fprintf(stderr, "%s\n", message);
        exit(3);
    }
    return !cond;
}

/*
 * Assert_nb return 0 if success, no error.
 */
int Assert_nb(int cond, char *message) {
    if (!cond) {
        fprintf(stderr, "%s\n", message);
    }
    return !cond;
}


int compare(const void *s1, const void *s2) {
    const struct dict_entry *e1 = s1;
    const struct dict_entry *e2 = s2;

    return strcmp(e1->str, e2->str);
}

int ut_get_line(char *buf, size_t size) {
    char *pos = memchr(buf, '\n', size);
    if (pos != NULL) {
        *(pos - 1) = 0;
        return 1;
    }
    return 0;
}

// TODO da testare bene
void ut_clone_configs(struct Configs* c, struct Configs* n){
    /*
    unsigned int port_number;
    char mode_concurrency;
    char *root_dir;
    int used_OPTARG;
    int* reset_config;
     */
    //int i;

    //*n = *c;

    // values
    n->port_number = c->port_number;
    n->mode_concurrency = c->mode_concurrency;
    n->used_OPTARG = c->used_OPTARG;


    //pointers
    // i = *(c->reset_config);
    //*(n->reset_config) = *(c->reset_config);

    n->root_dir = calloc(strlen(c->root_dir) +1, sizeof(char));
    strcpy(n->root_dir, c->root_dir);

    /*
     * code from main
   // values
   configs->port_number = 213;
   configs->mode_concurrency = M_PROCESS;
   configs->used_OPTARG = 0;
   configs->reset_config = NULL;
   configs->root_dir = "base";

   struct Configs c2;
   struct Configs* n;
   c2.reset_config = NULL ;
   n = &c2;
   n->port_number = 213;
   n->mode_concurrency = M_PROCESS;
   n->used_OPTARG = 0;
   n->reset_config = NULL;
   n->root_dir = "base";


   ut_clone_configs(configs, n);


   configs->port_number = 321;
   configs->mode_concurrency = M_THREAD;
   configs->used_OPTARG = 99;
   configs->reset_config = 0;
   configs->root_dir = "alterata";

   printf("struct n: port:%d mode:%d dir:%s\n", n->port_number, n->mode_concurrency, n->root_dir);
   printf("struct c: port:%d mode:%d dir:%s\n", configs->port_number, configs->mode_concurrency, configs->root_dir);
   */

}
