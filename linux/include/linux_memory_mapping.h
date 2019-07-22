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

#include "definitions.h"
int linux_memory_mapping(int fd_client, char* filename);
int files_memory_map(char* filename, int mode);
int linux_mmap(char *filename, int fd, void** addr);
#endif //GOPHER_LINUX_MEMORY_MAPPING_H
