//
// Created by andrea on 22/07/19.
//
#include <signal.h>
#include <stdio.h>

#include "linux_signals.h"
#include "definitions.h"
#include "config_file.h"
#include "linux_socket.h"




void signal_sighup_handler(int signo){

    if (signo == SIGHUP) {
        /*printf("received SIGINT\n");
        printf("prima del sig: %d\n", configs->port_number);
        conf_parseConfigFile("../gopher_server_configuration.txt", configs);
        printf("dopo il sig: %d\n", configs->port_number);*/
        int flagOne=1;
        (*configs).reset_config = &flagOne;


    }


}
