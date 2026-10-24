#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <termios.h>

#define FIONBIO     0x5421
#define FIONREAD    0x541B

int ioctl(int fd, unsigned long request, void *arg);

#ifdef __cplusplus
}
#endif
