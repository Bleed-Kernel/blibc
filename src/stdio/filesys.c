#include <libc/stdio.h>
#include <libc/errno.h>

int remove(const char *path) {
    (void)path;
    errno = ENOSYS;
    return -1;
}

int rename(const char *oldpath, const char *newpath) {
    (void)oldpath;
    (void)newpath;
    errno = ENOSYS;
    return -1;
}
