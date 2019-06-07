#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>
#include "definitions.h"
#include "utils.h"
#include "config_file.h"
#include "files_interaction.h"


// reformat Sh + ò
// comment Sh + ù
// run Sh + Enter
// Ctrl + Enter


int main(int argc, char *argv[]) {

    struct Configs configs;
    configs.root_dir = malloc(50 * sizeof(char));
    conf_parseConfigFile("../gopher_server_configuration.txt", &configs);

    // chiamata alla lettura del file di configurazione

    if(conf_read_opt(argc, argv, &configs) != 0) return 1;

    printf("port:%d mode:%d dir:|%s|\n", configs.port_number, configs.mode_concurrency, configs.root_dir);

    return 0;
}
