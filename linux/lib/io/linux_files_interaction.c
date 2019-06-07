//
// Created by Valerio Neri on 2019-06-07.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "definitions.h"
#include "utils.h"
#include "linux_files_interaction.h"
#include "linux_dict.h"


int compare(const void *s1, const void *s2) {
    const struct dict_entry *e1 = s1;
    const struct dict_entry *e2 = s2;

    return strcmp(e1->str, e2->str);
}

char check_subtype(char *arg) {

    /* sorted according to str */
    struct dict_entry dict[] = {
            {"directory",        '1'},
            {"gif",              'g'},
            {"octet-stream",     '9'},
            {"x-mach-binary",    '9'},
            {"x-pie-executable", '9'},
    };

    struct dict_entry *result, key = {arg};

    result = bsearch(&key, dict, sizeof(dict) / sizeof(dict[0]),
                     sizeof dict[0], compare);
    if (!result)
        return -1;

//    printf("Dict Result: %c\n", result->n);
    return result->n;
}


char check_type(char *arg) {

    /* sorted according to str */
    struct dict_entry dict[] = {
            {"image", 'I'},
    };

    struct dict_entry *result, key = {arg};

    result = bsearch(&key, dict, sizeof(dict) / sizeof(dict[0]),
                     sizeof dict[0], compare);
    if (!result)
        return -1;

//    printf("Dict Result: %d\n", result->n);
    return result->n;
}

// map
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#if __linux__
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
#define _MAP_POPULATE_AVAILABLE
#endif
#endif

#ifdef _MAP_POPULATE_AVAILABLE
#define MMAP_FLAGS (MAP_PRIVATE | MAP_POPULATE)
#else
#define MMAP_FLAGS MAP_PRIVATE
#endif
// end map



char getGopherCode(char* path){
    char* type;
    char* subtype;
    char* strtok_ptr;
    char gopher_code;

    char *command = concat("file -b --mime-type ", path);
    FILE *fp = popen(command, "r");

    if (!fp) {
        fprintf(stderr, "Could not open pipe for output.\n");
        return -1;
    }

//    printf("Exit code: %d\n", pclose(fp)/256);

    char *mime_type = NULL;
    size_t len = 0;

    if (getline(&mime_type, &len, fp) == -1) {
        fputs("file command failed", stderr);
        return -1;
    }

    char *file_not_found_respond = "cannot open `non-existing-file'";
//    printf("!!cmp:%s %d!!\n", mime_type, strncmp(file_not_found_respond, mime_type, strlen(file_not_found_respond)));
    if (strncmp(file_not_found_respond, mime_type, strlen(file_not_found_respond)) == 0) {
        return -2;
    }

    type = strtok_r(mime_type, "/", &strtok_ptr);
    subtype = strtok_r(NULL, "\n", &strtok_ptr);

//    printf("Code: %s | %s\n", type, subtype);

//    struct_test("kill", dict);

    if ((gopher_code = check_subtype(subtype)) == -1) {
        if ((gopher_code = check_type(type)) == -1) {
            gopher_code = '0';
        }
    }

    printf("GopherCode: %c\n", gopher_code);

    free(mime_type); // ?
    free(command);
    pclose(fp);

    return 0;
}

size_t getFilesize(const char *filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}




int mappLinux(char* filename, int mod){

    struct stat mystat;
    void* pmap;

    int fd = open(filename, O_RDWR);
    if( fd == -1 ) {
        perror("OpenError: ");
        exit(-1);
    }

    if (fstat(fd, &mystat) < 0){
        perror("fstat");
        close(fd);
        exit(1);
    }

    //Execute mmap

    size_t filesize = 894; //getFilesize(filename);

    // printf("%zu", filesize);

    pmap = mmap(0, mystat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (pmap == MAP_FAILED){
        perror("mmap");
        close(fd);
        exit(1);
    }

    close(fd);

    return 0;
    /*size_t filesize = getFilesize(filename);
    //Open file
    FILE* fd = fopen(filename, "r");
    if( fd == NULL ) {
        perror("Error: ");
        return(-1);
    }
    assert(fd != NULL);
    //Execute mmap

    void* mmappedData = mmap(NULL, filesize, PROT_READ, MMAP_FLAGS, fileno(fd), 0);
    assert(mmappedData != MAP_FAILED);
    //Write the mmapped data to stdout (= FD #1)
    write(1, mmappedData, filesize);
    //Cleanup
    int rc = munmap(mmappedData, filesize);
    assert(rc == 0);
    fclose(fd);
    return 0;*/
}
