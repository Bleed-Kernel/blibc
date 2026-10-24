#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <bleed/syscalls.h>
#include "compat_errno.h"

int unlink(const char *path) {
    if (!path) {
        errno = EINVAL;
        return -1;
    }
    long ret = _unlink(path);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return 0;
}

// the kernel unlinks either kind, this just keeps rmdir to folders
int rmdir(const char *path) {
    struct stat st;
    if (stat(path, &st) < 0)
        return -1;
    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }
    return unlink(path);
}

// there are no permissions, if it exists you can do what you like with it
int access(const char *path, int mode) {
    struct stat st;
    (void)mode;
    return stat(path, &st);
}

// the kernel has no fsync, just check the fd is real
int fsync(int fd) {
    struct stat st;
    return fstat(fd, &st);
}
