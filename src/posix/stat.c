#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include <bleed/syscalls.h>

#include "compat_errno.h"

static void from_kernel(const struct bleed_stat *k, struct stat *st) {
    memset(st, 0, sizeof(*st));
    st->st_dev     = (dev_t)k->dev;
    st->st_ino     = (ino_t)k->ino;
    st->st_mode    = (mode_t)k->mode;
    st->st_uid     = (uid_t)k->uid;
    st->st_gid     = (gid_t)k->gid;
    st->st_nlink   = (nlink_t)k->nlink;
    st->st_rdev    = (dev_t)k->rdev;
    st->st_size    = (off_t)k->size;
    st->st_blksize = (long)k->blksize;
    st->st_blocks  = (long)k->blocks;
    st->st_atim.tv_sec = (time_t)k->atime;
    st->st_mtim.tv_sec = (time_t)k->mtime;
    st->st_ctim.tv_sec = (time_t)k->ctime;
}

int stat(const char *path, struct stat *st) {
    struct bleed_stat k;
    long ret = _stat(path, &k);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    from_kernel(&k, st);
    return 0;
}

// theres no links to not follow
int lstat(const char *path, struct stat *st) {
    return stat(path, st);
}

int fstat(int fd, struct stat *st) {
    struct bleed_stat k;
    long ret = _fstat(fd, &k);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    from_kernel(&k, st);
    return 0;
}

int fstatat(int dirfd, const char *path, struct stat *st, int flags) {
    if ((flags & AT_EMPTY_PATH) && path && !*path)
        return fstat(dirfd, st);

    struct bleed_stat k;
    long ret = _fstatat(dirfd, path, &k, flags);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    from_kernel(&k, st);
    return 0;
}
