#include <sys/statvfs.h>

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <bleed/sysinfo.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

static size_t mount_covers(const char *mount, const char *path) {
    size_t len = strlen(mount);
    if (strcmp(mount, "/") == 0)
        return 1;
    if (strncmp(path, mount, len) != 0)
        return 0;
    return (path[len] == '\0' || path[len] == '/') ? len : 0;
}

int statvfs(const char *path, struct statvfs *buf) {
    char full[PATH_MAX];
    if (!path || !buf) {
        errno = EFAULT;
        return -1;
    }
    if (!realpath(path, full))
        return -1;

    // the longest mount path that covers it wins
    user_mount_info_t info, best;
    size_t best_len = 0;
    for (uint64_t i = 0; _mountinfo(i, &info) == 0; i++) {
        size_t len = mount_covers(info.path, full);
        if (len > best_len) {
            best_len = len;
            best = info;
        }
    }
    if (!best_len) {
        errno = ENOENT;
        return -1;
    }

    memset(buf, 0, sizeof(*buf));
    buf->f_bsize   = best.block_size;
    buf->f_frsize  = best.block_size;
    buf->f_blocks  = best.blocks_total;
    buf->f_bfree   = best.blocks_free;
    buf->f_bavail  = best.blocks_free;
    buf->f_namemax = 255;
    return 0;
}

// theres no way back from an fd to its path
int fstatvfs(int fd, struct statvfs *buf) {
    (void)fd;
    (void)buf;
    errno = ENOSYS;
    return -1;
}
