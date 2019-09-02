
#include <fcntl.h>
#include <errno.h>
#include "utils.h"

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

