#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <limits.h>

#define O_RDONLY 0x0
#define O_WRONLY 0x1
#define O_RDWR   0x2
#define O_MODE   0x3
#define O_ACCMODE 0x3
#define O_CREAT  0x40
#define O_CREATE O_CREAT // You're Welcome Ken Thompson, I did it. :pensive:
#define O_TRUNC  0x200
#define O_APPEND 0x400

int open(const char *path, int flags, ...);
int fcntl(int fd, int cmd, ...);

#define F_GETFL 3
#define F_SETFL 4

// all linux values, so code built for musl reads them right
#define O_NONBLOCK  0x800
#define O_NDELAY    O_NONBLOCK
#define O_NOCTTY    0x100
#define O_DIRECTORY 0x10000
#define O_CLOEXEC   0x80000
#define O_NOFOLLOW  0x20000
#define O_EXCL      0x80
#define O_SYNC      0x101000
#define O_DSYNC     0x1000
#define O_LARGEFILE 0x8000
#define O_NOATIME   0x40000
#define O_PATH      0x200000
#define O_TMPFILE   0x410000

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define FD_CLOEXEC 1
#define F_DUPFD_CLOEXEC 1030

// record locks, theres one user so every lock is granted
#define F_GETLK   5
#define F_SETLK   6
#define F_SETLKW  7
#define F_OFD_GETLK  36
#define F_OFD_SETLK  37
#define F_OFD_SETLKW 38
#define F_RDLCK   0
#define F_WRLCK   1
#define F_UNLCK   2
#define F_SETOWN  8
#define F_GETOWN  9

struct flock {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};
#define F_ADD_SEALS     1033
#define F_GET_SEALS     1034

#define F_SEAL_SEAL         0x1
#define F_SEAL_SHRINK       0x2
#define F_SEAL_GROW         0x4
#define F_SEAL_WRITE        0x8
#define F_SEAL_FUTURE_WRITE 0x10

#define AT_FDCWD            (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR        0x200
#define AT_SYMLINK_FOLLOW   0x400
#define AT_EMPTY_PATH       0x1000
#define AT_EACCESS          0x200
#define AT_NO_AUTOMOUNT     0x800

int openat(int dirfd, const char *path, int flags, ...);

#define POSIX_FADV_NORMAL     0
#define POSIX_FADV_RANDOM     1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED   3
#define POSIX_FADV_DONTNEED   4
#define POSIX_FADV_NOREUSE    5

// hints, the vfs has no readahead to tell
ssize_t readahead(int fd, off_t offset, size_t count);
int     posix_fadvise(int fd, off_t offset, off_t len, int advice);
int     posix_fallocate(int fd, off_t offset, off_t len);

#define FALLOC_FL_KEEP_SIZE  0x01
#define FALLOC_FL_PUNCH_HOLE 0x02
// mode 0 grows the file like posix_fallocate, the rest say EOPNOTSUPP
int     fallocate(int fd, int mode, off_t offset, off_t len);
int unlinkat(int dirfd, const char *path, int flags);

#ifdef __cplusplus
}
#endif
