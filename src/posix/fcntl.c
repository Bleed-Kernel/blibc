#include <fcntl.h>

#include <stdarg.h>
#include <bleed/console.h>
#include <errno.h>
#include <bleed/syscalls.h>
#include <sys/stat.h>

#include "compat_errno.h"

int open(const char *path, int flags, ...) {
    long ret = _open(path, flags);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int openat(int dirfd, const char *path, int flags, ...) {
    long ret = _openat(dirfd, path, flags);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int fcntl(int fd, int cmd, ...) {
    va_list ap;
    int flags = 0;
    int in_flags = 0;

    switch (cmd) {
    // nothing is ever closed on exec, so the only descriptor flag reads as unset
    case F_GETFD:
    case F_SETFD:
        return 0;
    case F_DUPFD: {
        va_start(ap, cmd);
        int lowest = va_arg(ap, int);
        va_end(ap);

        struct bleed_stat st;
        for (int target = lowest; target < 256; target++) {
            if (_fstat(target, &st) == -EBADF) {
                long ret = _dup2(fd, target);
                return ret < 0 ? (int)posix_errno_from_ret(ret) : (int)ret;
            }
        }
        errno = EMFILE;
        return -1;
    }
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
