#include <stdio.h>
#include <utils.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include "protocol.h"
#include "definitions.h"

#if defined(__unix__) || defined(__APPLE__)

#include "linux_protocol.h"

#endif
#ifdef _WIN32

#include "windows_protocol.h"

#endif

#define GM_MAX_LINESIZE 200
#define GM_MIN_LINESIZE 5

int resolve_selector(char *gopher_root, char **filepath, const char *selector) {
    //char* gopher_root = "/opt/local_projects/gopher-project/";
    *filepath = calloc(strlen(gopher_root) + 1 + strlen(selector) + 4, sizeof(char));

    if (*filepath == NULL) {
        perror("calloc fail in protocol.c/resolve_selector");
        return NO_FREE;
    }

    int ret;
    if (gopher_root[strlen(gopher_root) - 1] == OS_SEPARATOR) {
        ret = sprintf(*filepath, "%s%s", gopher_root, selector + (selector[0] == OS_SEPARATOR ? 1 : 0));
    } else {
        ret = sprintf(*filepath, "%s%c%s", gopher_root, OS_SEPARATOR, selector + (selector[0] == OS_SEPARATOR ? 1 : 0));
    }

    if (ret < 0) {
        perror("sprintf fail in protocol.c/resolve_selector");
        return NEED_TO_FREE;
    }
    return 0;
}

int protocol_response(char type, char *filename, char *path, const char *host, int port, char **result) {


    if (type == '3') { // ERROR type 3
        // format
        // 3 '/gopher/clients/sdas.txt' doesn't exist!		error.host	1
        char *error = " doesn't exist! error.host";
        *result = calloc(//result,
                (1 /*char len*/ + strlen("" /*path*/) + strlen(filename) + strlen(error) + 6 /*port len*/ +
                 4 /*spaces*/ + 4 /*extra space*/), sizeof(char));
        if (*result == NULL) {
            return 1;
        }
        int ret = sprintf(*result, "%c%s%s\t%s\t%d\n", '3', "" /*path*/, filename, error, port);
        if (ret < 0) {
            perror("sprintf fail in protocol.c/resolve_selector");
            return NEED_TO_FREE;
        }
    } else {
        // Format for directory listening

        *result = calloc(//result,
                (1 /*type len*/ + strlen(path) + strlen(filename) + strlen(host) + 6 /*port len*/ +
                 4 /*spaces*/ + 4 /*extra space*/), sizeof(char));
        if (*result == NULL) {
            return 1;
        }
        if (sprintf(*result, "%c%s\t%s\t%s\t%d\n", type, filename, path, host, port) < 0) {
            return NEED_TO_FREE;
        }
        printf("protocol_response/result %s\n", *result);
    }
    return 0;
}

//static bool is_valid_gopher_line(const char *line) {
//    return (strlen(line) >= GM_MIN_LINESIZE && (
//            (line[0] >= '0' && line[0] <= '9') ||
//            (line[0] >= 'A' && line[0] <= 'Z') ||
//            (line[0] >= 'a' && line[0] <= 'z')) &&
//            (strchr(line, '\t') || line[0] == 'i'));
//}


int print_directory(char *path, int (*socket_send_f)(int, char *), int fd, int port) {

    struct dirent *entry = NULL;

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error:print_directory/opendir:");
        return -1;
    }

    size_t pathlen = strlen(path);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // don't print hidden files/dirs
        char *filename = entry->d_name;

        //printf("%s\n", "Fin qui si");


        char *fullpath = calloc(pathlen + strlen(filename) + 5, sizeof(char));
        if (fullpath == NULL){
            return -1;
        }
        int err = sprintf(fullpath, "%s/%s", path, filename);

        if (err < 0) {
            //linux_sock_send_error(fd);
            perror("Error:print_directory/sprintf");
            free(fullpath);

            return -1;
        }

        char code = getGopherCode(fullpath);

        // get line to send for gopher
        char *response_line;
        err = protocol_response(code, filename, fullpath, "localhost", port, &response_line);

        if (err != 0) {

            perror("Error:print_directory:protocol_response");
            free(fullpath);
            if (err == NEED_TO_FREE){
                free(response_line);
            }

            return -1;
        }

        // send line
        int ret = (*socket_send_f)(fd, response_line);

        free(fullpath);
        free(response_line);

        if (0 > ret) {
            return -2;
        }

    }

    if (closedir(dir) != 0) perror("Error:print_directory:close_dir");

    return 0;
}



