#include <libc/unistd.h>

#include <libc/devices/console.h>
#include <libc/errno.h>
#include <libc/syscalls/chdir.h>
#include <libc/syscalls/getcwd.h>
#include <libc/syscalls/ioctl.h>
#include <libc/syscalls/read.h>
#include <libc/syscalls/seek.h>
#include <libc/syscalls/write.h>
#include <libc/syscalls/close.h>

#include "compat_errno.h"

ssize_t read(int fd, void *buf, size_t len) {
    long ret = _read(fd, buf, (unsigned long)len);
    if (ret < 0) return posix_errno_from_ret(ret);
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t len) {
    long ret = _write(fd, buf, (long)len);
    if (ret < 0) return posix_errno_from_ret(ret);
    return (ssize_t)ret;
}

int close(int fd) {
    long ret = _close(fd);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

off_t lseek(int fd, off_t offset, int whence) {
    long ret = _seek(fd, (long)offset, whence);
    if (ret < 0) return (off_t)posix_errno_from_ret(ret);
    return (off_t)ret;
}

int chdir(const char *path) {
    long ret = _chdir(path);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

char *getcwd(char *buf, size_t size) {
    char *ret = _getcwd(buf, (long)size);
    if (!ret) {
        errno = EIO;
        return NULL;
    }
    return ret;
}

int isatty(int fd) {
    int flags = 0;
    long ret = _ioctl(fd, TTY_IOCTL_GET_FLAGS, &flags);
    if (ret < 0) {
        errno = ENOTTY;
        return 0;
    }
    return 1;
}
