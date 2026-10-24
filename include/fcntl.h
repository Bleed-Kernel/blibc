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
#define O_CREAT  0x4
#define O_CREATE 0x4 // You're Welcome Ken Thompson, I did it. :pensive:
#define O_TRUNC  0x8
#define O_APPEND 0x10

int open(const char *path, int flags, ...);
int fcntl(int fd, int cmd, ...);

#define F_GETFL 3
#define F_SETFL 4

#define O_NONBLOCK 0x20
#define O_NOCTTY    0x40
#define O_DIRECTORY 0x80
#define O_CLOEXEC   0x100
#define O_NOFOLLOW  0x200

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define FD_CLOEXEC 1

#define AT_FDCWD            (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_EMPTY_PATH       0x1000

int openat(int dirfd, const char *path, int flags, ...);

#ifdef __cplusplus
}
#endif
