#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <bleed/syscalls.h>
#include "compat_errno.h"

#define DIRENT_DIR  0

int unlink(const char *path) {
    if (!path) {
        errno = EINVAL;
        return -1;
    }
    long ret = _unlink(path);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return 0;
}

// stat cant say whats a folder yet
static int is_dir(const char *path) {
    char parent[PATH_MAX];
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/')
        len--;
    if (len >= sizeof(parent))
        return -1;
    memcpy(parent, path, len);
    parent[len] = '\0';

    char *slash = strrchr(parent, '/');
    char  want[256];
    strncpy(want, slash ? slash + 1 : parent, sizeof(want) - 1);
    want[sizeof(want) - 1] = '\0';

    if (!slash)
        strcpy(parent, ".");
    else if (slash == parent)
        parent[1] = '\0';
    else
        *slash = '\0';

    int fd = _open(parent, 0);
    if (fd < 0)
        return -1;
    dirent_t entry;
    int found = -1;
    for (size_t i = 0; _readdir(fd, i, &entry) >= 0 && entry.name[0]; i++) {
        if (strcmp(entry.name, want) == 0) {
            found = entry.type == DIRENT_DIR;
            break;
        }
    }
    _close(fd);
    return found;
}

int rmdir(const char *path) {
    if (!path) {
        errno = EINVAL;
        return -1;
    }
    int dir = is_dir(path);
    if (dir < 0) {
        errno = ENOENT;
        return -1;
    }
    if (!dir) {
        errno = ENOTDIR;
        return -1;
    }
    return unlink(path);
}
