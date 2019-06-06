//
// Created by andreabacciu on 6/6/19.
//

#ifndef PROJECT_CONFIG_FILE_H
#define PROJECT_CONFIG_FILE_H

#endif //PROJECT_CONFIG_FILE_H

#include "definitions.h"

int conf_opts_port_number(char* opt);

char conf_opts_mode_concurrency(char* opt);

char* conf_opts_root_dir(char* opt);

int conf_read_opt(int argc, char *argv[], struct Configs* configs);

int conf_parseConfigFile(char *path, struct Configs *config);
