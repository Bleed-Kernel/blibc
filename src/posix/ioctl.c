#include <sys/ioctl.h>

#include <bleed/syscalls.h>

#include "compat_errno.h"

int ioctl(int fd, unsigned long request, void *arg) {
    long ret = _ioctl(fd, request, arg);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}
