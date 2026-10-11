#pragma once

#include <sys/types.h>
#include <limits.h>

#define O_RDONLY 0x0
#define O_WRONLY 0x1
#define O_RDWR   0x2
#define O_MODE   0x3
#define O_CREAT  0x4
#define O_TRUNC  0x8
#define O_APPEND 0x10

int open(const char *path, int flags, ...);
int fcntl(int fd, int cmd, ...);

#define F_GETFL 3
#define F_SETFL 4

#define O_NONBLOCK 0x20
