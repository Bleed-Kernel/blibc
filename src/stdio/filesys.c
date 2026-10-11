#include <stdio.h>
#include <errno.h>
#include <bleed/syscalls.h>

int remove(const char *path) {
    if (!path) {
        errno = EINVAL;
        return -1;
    }

    int ret = _unlink(path);
    if (ret < 0) {
        long code = -ret;
        errno = (code > 0 && code <= 4095) ? (int)code : EIO;
        return -1;
    }

    return 0;
}

int rename(const char *oldpath, const char *newpath) {
    if (!oldpath || !newpath) {
        errno = EINVAL;
        return -1;
    }

    int ret = _rename(oldpath, newpath);
    if (ret < 0) {
        long code = -ret;
        errno = (code > 0 && code <= 4095) ? (int)code : EIO;
        return -1;
    }

    return 0;
}
