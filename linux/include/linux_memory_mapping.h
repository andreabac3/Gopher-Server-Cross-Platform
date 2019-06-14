//
// Created by Andrea Bacciu on 2019-06-07.
//

// map
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#if __linux__
#include <sys/mman.h>
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

#ifndef GOPHER_LINUX_MEMORY_MAPPING_H
#define GOPHER_LINUX_MEMORY_MAPPING_H


int files_memory_map(char* filename, int mode);










#endif //GOPHER_LINUX_MEMORY_MAPPING_H
