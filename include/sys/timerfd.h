#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>

#define TFD_TIMER_ABSTIME   0x1
#define TFD_NONBLOCK        0x800
#define TFD_CLOEXEC         0x80000

int timerfd_create(int clock, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec *value, struct itimerspec *old);
int timerfd_gettime(int fd, struct itimerspec *cur);

#ifdef __cplusplus
}
#endif
