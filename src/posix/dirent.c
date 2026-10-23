#include <dirent.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <bleed/syscalls.h>
#include <sys/stat.h>

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

DIR *fdopendir(int fd) {
    struct stat st;
    if (fstat(fd, &st) < 0)
        return NULL;
    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return NULL;
    }

    DIR *dirp = (DIR *)malloc(sizeof(DIR));
    if (!dirp) {
        errno = ENOMEM;
        return NULL;
    }

    dirp->fd = fd;
    dirp->index = 0;
    dirp->entry.d_ino = 0;
    dirp->entry.d_type = 0;
    dirp->entry.d_name[0] = '\0';
    return dirp;
}

int dirfd(DIR *dirp) {
    if (!dirp) {
        errno = EINVAL;
        return -1;
    }
    return dirp->fd;
}

static unsigned char dirent_type(int kernel_type) {
    switch (kernel_type) {
    case 0:  return DT_DIR;
    case 1:  return DT_REG;
    case 2:  return DT_CHR;
    default: return DT_UNKNOWN;
    }
}

struct dirent *readdir(DIR *dirp) {
    dirent_t raw;
    long ret;

    if (!dirp) {
        errno = EINVAL;
        return NULL;
    }

    ret = _readdir(dirp->fd, (size_t)dirp->index, &raw);
    if (ret == -ENOENT) return NULL;
    if (ret < 0) {
        posix_errno_from_ret(ret);
        return NULL;
    }

    dirp->index++;
    dirp->entry.d_ino = (ino_t)dirp->index;
    dirp->entry.d_type = dirent_type(raw.type);
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
