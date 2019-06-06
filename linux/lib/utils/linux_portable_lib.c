//
// Created by andreabacciu on 6/6/19.
//

#include "linux_portable_lib.h"
#include <zconf.h>

int __cd(char* path){
    return chdir(path);
}





