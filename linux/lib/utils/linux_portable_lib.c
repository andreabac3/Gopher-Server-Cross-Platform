//
// Created by andreabacciu on 6/6/19.
//

#include <unistd.h>
#include "linux_portable_lib.h"

int __cd(char* path){
    return chdir(path);
}





