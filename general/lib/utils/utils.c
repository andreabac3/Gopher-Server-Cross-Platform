//
// Created by valerioneri on 6/5/19.
//

#include <stdio.h>
#include "utils.h"
#include <stdlib.h>

#include "config_file.h"

#ifdef __unix__

#include "gopher_server_configuration_linux.h"

#endif

void print(int i) {
    printf("%d\n", i);
}

int mul(int a) {
    return a * 2;
}


