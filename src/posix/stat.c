#include <sys/stat.h>

#include <bleed/syscalls.h>

#include "compat_errno.h"

int stat(const char *path, struct stat *st) {
    long ret = _stat(path, st);
    if (ret < 0) {
        return (int)posix_errno_from_ret(ret);
    }
    return 0;
}

// theres no links to not follow
int lstat(const char *path, struct stat *st) {
    return stat(path, st);
}

int fstat(int fd, struct stat *st) {
    long ret = _fstat(fd, st);
    if (ret < 0) {
        return (int)posix_errno_from_ret(ret);
    }
    return 0;
}
