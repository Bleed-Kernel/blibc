#include <libc/errno.h>
#include <libc/sys/stat.h>

int mkdir(const char *path, mode_t mode) {
    (void)path;
    (void)mode;
    errno = ENOSYS;
    return -1;
}
