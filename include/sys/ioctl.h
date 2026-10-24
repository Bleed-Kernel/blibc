#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <termios.h>

int ioctl(int fd, unsigned long request, void *arg);

#ifdef __cplusplus
}
#endif
