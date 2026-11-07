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
    case F_GETFD:
    case F_SETFD:
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
    case F_ADD_SEALS:
    case F_GET_SEALS: {
        va_start(ap, cmd);
        long arg = (cmd == F_GETFD || cmd == F_GET_SEALS) ? 0 : va_arg(ap, int);
        va_end(ap);
        long ret = _fcntl(fd, cmd, arg);
        return ret < 0 ? (int)posix_errno_from_ret(ret) : (int)ret;
    }
    case F_SETLK:
    case F_SETLKW:
    case F_OFD_SETLK:
    case F_OFD_SETLKW:
        return 0;
    case F_GETLK:
    case F_OFD_GETLK: {
        va_start(ap, cmd);
        struct flock *fl = va_arg(ap, struct flock *);
        va_end(ap);
        if (fl)
            fl->l_type = F_UNLCK;
        return 0;
    }
    case F_SETOWN:
        return 0;
    case F_GETOWN:
        return 0;
    case F_GETFL: {
        // the kernel knows how a file was opened, a tty keeps its own nonblock flag
        long ret = _fcntl(fd, F_GETFL, 0);
        if (ret < 0)
            return (int)posix_errno_from_ret(ret);
        in_flags = (int)ret;
        if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &flags) == 0 && (flags & TTY_NONBLOCK))
            in_flags |= O_NONBLOCK;
        return in_flags;
    }
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
