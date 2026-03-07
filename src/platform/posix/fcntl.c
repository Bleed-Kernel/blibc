#include <libc/fcntl.h>

#include <stdarg.h>
#include <libc/devices/console.h>
#include <libc/errno.h>
#include <libc/syscalls/ioctl.h>
#include <libc/syscalls/open.h>

#include "compat_errno.h"

int open(const char *path, int flags, ...) {
    long ret = _open(path, flags);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int fcntl(int fd, int cmd, ...) {
    va_list ap;
    int flags = 0;
    int in_flags = 0;

    switch (cmd) {
    case F_GETFL:
        if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &flags) < 0) {
            errno = ENOTTY;
            return -1;
        }
        if (flags & TTY_NONBLOCK) in_flags |= O_NONBLOCK;
        return in_flags;
    case F_SETFL: {
        int nonblock = 0;
        va_start(ap, cmd);
        in_flags = va_arg(ap, int);
        va_end(ap);

        nonblock = (in_flags & O_NONBLOCK) ? 1 : 0;
        if (_ioctl(fd, TTY_IOCTL_FIONBIO, &nonblock) == 0)
            return 0;

        if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &flags) < 0) {
            errno = ENOTTY;
            return -1;
        }
        if (in_flags & O_NONBLOCK) flags |= TTY_NONBLOCK;
        else flags &= ~TTY_NONBLOCK;
        if (_ioctl(fd, TTY_IOCTL_SET_FLAGS, &flags) < 0) {
            errno = ENOTTY;
            return -1;
        }
        return 0;
    }
    default:
        errno = ENOSYS;
        return -1;
    }
}
