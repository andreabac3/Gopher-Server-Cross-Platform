#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

//#include "utils.h"

// reformat Sh + ò
// comment Sh + ù
// run Sh + Enter
// Ctrl + Enter

struct Configs {
    int  port_number;
    char*  mode_concurrency;
    char*  root_dir;
} configs;

void help(){
    char* helpString = "";
    printf("%s\n", helpString);
}

int main(int argc, char *argv[]) {

    int opt;

    configs.root_dir = malloc(50 * sizeof(char));

    // chiamata alla lettura del file di configurazione

    while((opt = getopt(argc, argv, "m:d:p:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                printf("%s \n", optarg);
                configs.port_number = atoi(optarg);
                break;
            case 'm':
                assert(strcmp(optarg, "Thread") != 0 || strcmp(optarg, "Process") != 0);
                configs.mode_concurrency = optarg;
                break;
            case 'd':
                configs.root_dir = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                return 1;
        }
    }

    printf("%d %s %s\n", configs.port_number, configs.mode_concurrency, configs.root_dir);


    printf("Hello, World!\n");
    // print(mul2(4, 5));
    return 0;
}
