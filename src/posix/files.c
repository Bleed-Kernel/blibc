#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <bleed/syscalls.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/time.h>
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

int unlinkat(int dirfd, const char *path, int flags) {
    long ret;
    asm volatile("syscall" : "=a"(ret) : "a"(263), "D"((long)dirfd), "S"(path), "d"((long)flags) : "rcx", "r11", "memory");
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return 0;
}

int truncate(const char *path, off_t length) {
    int fd = open(path, O_WRONLY);
    if (fd < 0)
        return -1;
    int r = ftruncate(fd, length);
    int saved = errno;
    close(fd);
    errno = saved;
    return r;
}

static int exists(const char *path) {
    struct stat st;
    return stat(path, &st);
}

static int fd_exists(int fd) {
    struct stat st;
    return fstat(fd, &st);
}

int link(const char *from, const char *to) {
    (void)to;
    if (exists(from) < 0)
        return -1;
    errno = EPERM;
    return -1;
}

int linkat(int fromfd, const char *from, int tofd, const char *to, int flags) {
    (void)fromfd; (void)tofd; (void)flags;
    return link(from, to);
}

int symlink(const char *target, const char *path) {
    return symlinkat(target, AT_FDCWD, path);
}

int symlinkat(const char *target, int dirfd, const char *path) {
    if (!target || !path) {
        errno = EINVAL;
        return -1;
    }
    long ret = _symlinkat(target, dirfd, path);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return 0;
}

ssize_t readlinkat(int dirfd, const char *path, char *buf, size_t size) {
    // /proc/self/exe isnt a real link, readlink makes it up from the task path
    if (path && !strncmp(path, "/proc/", 6))
        return readlink(path, buf, size);
    long ret = _readlinkat(dirfd, path, buf, size);
    if (ret < 0) return (ssize_t)posix_errno_from_ret(ret);
    return (ssize_t)ret;
}

int chown(const char *path, uid_t uid, gid_t gid) {
    (void)uid; (void)gid;
    return exists(path);
}

int lchown(const char *path, uid_t uid, gid_t gid) {
    return chown(path, uid, gid);
}

int fchown(int fd, uid_t uid, gid_t gid) {
    (void)uid; (void)gid;
    return fd_exists(fd);
}

int chmod(const char *path, mode_t mode) {
    (void)mode;
    return exists(path);
}

int fchmod(int fd, mode_t mode) {
    (void)mode;
    return fd_exists(fd);
}

int fchmodat(int dirfd, const char *path, mode_t mode, int flags) {
    struct stat st;
    (void)mode; (void)flags;
    return fstatat(dirfd, path, &st, 0);
}

mode_t umask(mode_t mask) {
    (void)mask;
    return 022;
}

int utimensat(int dirfd, const char *path, const struct timespec times[2], int flags) {
    struct stat st;
    (void)times; (void)flags;
    return fstatat(dirfd, path, &st, 0);
}

int futimens(int fd, const struct timespec times[2]) {
    (void)times;
    return fd_exists(fd);
}

int utimes(const char *path, const struct timeval times[2]) {
    (void)times;
    return exists(path);
}

int utime(const char *path, const struct utimbuf *times) {
    (void)times;
    return exists(path);
}

long pathconf(const char *path, int name) {
    if (exists(path) < 0)
        return -1;
    return fpathconf(-1, name);
}

long fpathconf(int fd, int name) {
    (void)fd;
    switch (name) {
        case _PC_LINK_MAX:  return 1;
        case _PC_MAX_CANON:
        case _PC_MAX_INPUT: return 255;
        case _PC_NAME_MAX:  return 255;
        case _PC_PATH_MAX:  return 4096;
        case _PC_PIPE_BUF:  return 4096;
        default:
            errno = EINVAL;
            return -1;
    }
}

pid_t setsid(void) {
    return getpid();
}

pid_t getsid(pid_t pid) {
    return pid ? pid : getpid();
}

int setpgid(pid_t pid, pid_t pgid) {
    (void)pid;
    (void)pgid;
    return 0;
}

pid_t getpgid(pid_t pid) {
    return pid ? pid : getpid();
}

pid_t getpgrp(void) {
    return getpid();
}
