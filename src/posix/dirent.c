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
    dirp->snap = NULL;
    dirp->count = 0;
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
    dirp->snap = NULL;
    dirp->count = 0;
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
    case 3:  return DT_LNK;
    default: return DT_UNKNOWN;
    }
}

struct __dirsnap {
    unsigned char type;
    char name[256];
};

static int read_listing(DIR *dirp) {
    unsigned long cap = 16, n = 0;
    struct __dirsnap *snap = malloc(cap * sizeof(*snap));
    if (!snap) {
        errno = ENOMEM;
        return -1;
    }

    for (;;) {
        dirent_t raw;
        long ret = _readdir(dirp->fd, (size_t)n, &raw);
        if (ret == -ENOENT)
            break;
        if (ret < 0) {
            free(snap);
            posix_errno_from_ret(ret);
            return -1;
        }
        if (n == cap) {
            struct __dirsnap *bigger = realloc(snap, cap * 2 * sizeof(*snap));
            if (!bigger) {
                free(snap);
                errno = ENOMEM;
                return -1;
            }
            snap = bigger;
            cap *= 2;
        }
        snap[n].type = dirent_type(raw.type);
        strncpy(snap[n].name, raw.name, sizeof(snap[n].name) - 1);
        snap[n].name[sizeof(snap[n].name) - 1] = '\0';
        n++;
    }

    dirp->snap  = snap;
    dirp->count = n;
    return 0;
}

struct dirent *readdir(DIR *dirp) {
    if (!dirp) {
        errno = EINVAL;
        return NULL;
    }
    if (!dirp->snap && read_listing(dirp) < 0)
        return NULL;
    if (dirp->index >= dirp->count)
        return NULL;

    struct __dirsnap *e = &dirp->snap[dirp->index++];
    dirp->entry.d_ino = (ino_t)dirp->index;
    dirp->entry.d_off = (off_t)dirp->index;
    dirp->entry.d_reclen = (unsigned short)sizeof(dirp->entry);
    dirp->entry.d_type = e->type;
    memcpy(dirp->entry.d_name, e->name, sizeof(dirp->entry.d_name));
    return &dirp->entry;
}

int closedir(DIR *dirp) {
    int ret = 0;

    if (!dirp) {
        errno = EINVAL;
        return -1;
    }

    ret = _close(dirp->fd);
    free(dirp->snap);
    free(dirp);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return 0;
}

void rewinddir(DIR *dirp) {
    if (!dirp) return;
    free(dirp->snap);
    dirp->snap  = NULL;
    dirp->count = 0;
    dirp->index = 0;
}
