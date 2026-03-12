#include <libc/errno.h>
#include <libc/sys/stat.h>
#include <libc/syscalls/mkdir.h>

int mkdir(const char *path, mode_t mode) {
    if (!path) {
        errno = EINVAL;
        return -1;
    }

    int ret = _mkdir(path, (int)mode);
    if (ret < 0) {
        long code = -ret;
        errno = (code > 0 && code <= 4095) ? (int)code : EIO;
        return -1;
    }

    return 0;
}
