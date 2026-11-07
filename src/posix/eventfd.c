#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <errno.h>

#include "compat_errno.h"

static long fd_syscall(long n, long a, long b, long c, long d) {
    long ret;
    register long r10 asm("r10") = d;
    asm volatile("syscall" : "=a"(ret) : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10) : "rcx", "r11", "memory");
    return ret;
}

static int fd_ret(long r) {
    return r < 0 ? (int)posix_errno_from_ret(r) : (int)r;
}

int eventfd(unsigned int initval, int flags) {
    return fd_ret(fd_syscall(290, initval, flags, 0, 0));
}

int eventfd_read(int fd, eventfd_t *value) {
    return read(fd, value, sizeof(*value)) == sizeof(*value) ? 0 : -1;
}

int eventfd_write(int fd, eventfd_t value) {
    return write(fd, &value, sizeof(value)) == sizeof(value) ? 0 : -1;
}

int timerfd_create(int clock, int flags) {
    return fd_ret(fd_syscall(283, clock, flags, 0, 0));
}

int timerfd_settime(int fd, int flags, const struct itimerspec *value, struct itimerspec *old) {
    return fd_ret(fd_syscall(286, fd, flags, (long)value, (long)old));
}

int timerfd_gettime(int fd, struct itimerspec *cur) {
    return fd_ret(fd_syscall(287, fd, (long)cur, 0, 0));
}
