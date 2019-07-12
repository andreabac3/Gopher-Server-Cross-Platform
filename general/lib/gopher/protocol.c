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

int resolve_selector(char* gopher_root, char** filepath, const char *selector) {
    //char* gopher_root = "/opt/local_projects/gopher-project/";
    *filepath = calloc( strlen(gopher_root) + 1 + strlen(selector) + 4 , sizeof(char));
    if (*filepath == NULL){
        perror("calloc fail in protocol.c/resolve_selector");
        return NO_FREE;
    }
    int ret = sprintf(*filepath, "%s%c%s", gopher_root, OS_SEPARATOR, selector + (selector[0] == '/' ? 1 : 0));
    if (ret < 0) {
        perror("sprintf fail in protocol.c/resolve_selector");
        return NEED_TO_FREE;
    }
    return 0;
}

int protocol_response(char type, char *filename, char *path, const char *host, int port, char **result) {
//    int ret = Assert_nb(0 < type && type < 10, "protocol_response -> Error invalid type.");
//    ret |= Assert_nb(0 < port && port < 65536, "protocolResponse2 -> Error invalid port");
//    if (ret > 0) {
//        return 2;
//    }
    if (type == '3') { // ERROR type 3
        // format
        // 3 '/gopher/clients/sdas.txt' doesn't exist!		error.host	1
        char *error = " doesn't exist! error.host";
        *result = malloc(//result,
                         (1 /*char len*/ + strlen(path) + strlen(filename) + strlen(error) + 6 /*port len*/ +
                          4 /*spaces*/ +
                          4 /*extra space*/) * sizeof(char));
        if (*result == NULL) {
            return 1;
        }
        sprintf(*result, "%c%s%s\t%s\t%d\n", '3', path, filename, error, port);
    } else {
        // format
        // type + filename + relative_path + host + port
        *result = malloc(//result,
                         (1 /*type len*/ + strlen(path) + strlen(filename) + strlen(host) + 6 /*port len*/ +
                          4 /*spaces*/ +
                          4 /*extra space*/) * sizeof(char));
        if (*result == NULL) {
            return 1;
        }
        sprintf(*result, "%c%s\t%s\t%s\t%d\n", type, filename, path, host, port);
        printf("protocol_response/result %s\n", *result);
    }
    return 0;
}

static bool is_valid_gopher_line(const char *line) {
    return (strlen(line) >= GM_MIN_LINESIZE && (
            (line[0] >= '0' && line[0] <= '9') ||
            (line[0] >= 'A' && line[0] <= 'Z') ||
            (line[0] >= 'a' && line[0] <= 'z')) &&
            (strchr(line, '\t') || line[0] == 'i'));
}


int print_directory(char *path, void (*socket_send_f)(int *, char *), int *fd) {
    DIR *dir = opendir(path);
    struct dirent *entry = NULL;

    if (dir == NULL) {
        perror("print_directory/opendir:");
        return -1;
    }

    printf("%s\n", "dir opened");

    size_t pathlen = strlen(path);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // don't print hidden files/dirs
        char *filename = entry->d_name;

        printf("filename: %s\n", filename);

        //printf("%s\n", "Fin qui si");

        char *fullpath = calloc(pathlen + strlen(filename) + 5, sizeof(char));
        int err = sprintf(fullpath, "%s/%s", path, filename);

        if (err < 0) {
            //linux_sock_send_error(fd);
            perror("Error:print_directory/sprintf");
            free(fullpath);

            return -1;
        }

        char code = getGopherCode(fullpath);
        //err |= (code < 0);

        printf("gopgerCode %c\n", code);

        // get line for gopher
        char *line;
        err = protocol_response(code, filename, fullpath, "localhost", 7070, &line);

        if (err != 0) {
            //linux_sock_send_error(fd);

            perror("Error:print_directory");
            free(fullpath);
            free(line);

            return -1;
        }

        printf("line: %s\n", line);
        printf("is_valid_gopher_line: %d\n", is_valid_gopher_line(line));
        printf("sockfd: %d\n", *fd);

        // send line
        (*socket_send_f)(fd, line);


        free(fullpath);
        free(line);

    }

    if (closedir(dir) != 0)
        perror(NULL);

    return 0;
}



