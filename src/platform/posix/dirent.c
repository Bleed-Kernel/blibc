#include <libc/dirent.h>

#include <libc/errno.h>
#include <libc/fcntl.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/syscalls/close.h>
#include <libc/syscalls/open.h>
#include <libc/syscalls/readdir.h>

#include "compat_errno.h"

DIR *opendir(const char *name) {
    long fd;
    DIR *dirp;

    if (!name) {
        errno = EINVAL;
        return NULL;
    }

    fd = _open(name, O_RDONLY);
    if (fd < 0) {
        posix_errno_from_ret(fd);
        return NULL;
    }

    dirp = (DIR *)malloc(sizeof(DIR));
    if (!dirp) {
        _close((int)fd);
        errno = ENOMEM;
        return NULL;
    }

    dirp->fd = (int)fd;
    dirp->index = 0;
    dirp->entry.d_ino = 0;
    dirp->entry.d_type = 0;
    dirp->entry.d_name[0] = '\0';
    return dirp;
}

struct dirent *readdir(DIR *dirp) {
    dirent_t raw;
    long ret;

    if (!dirp) {
        errno = EINVAL;
        return NULL;
    }

    ret = _readdir(dirp->fd, (size_t)dirp->index, &raw);
    if (ret < 0) {
        posix_errno_from_ret(ret);
        return NULL;
    }
    if (ret == 0) return NULL;

    dirp->index++;
    dirp->entry.d_ino = (ino_t)dirp->index;
    dirp->entry.d_type = (unsigned char)raw.type;
    strncpy(dirp->entry.d_name, raw.name, sizeof(dirp->entry.d_name) - 1);
    dirp->entry.d_name[sizeof(dirp->entry.d_name) - 1] = '\0';
    return &dirp->entry;
}

int closedir(DIR *dirp) {
    int ret = 0;

    if (!dirp) {
        errno = EINVAL;
        return -1;
    }

    ret = _close(dirp->fd);
    free(dirp);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return 0;
}

void rewinddir(DIR *dirp) {
    if (!dirp) return;
    dirp->index = 0;
}
