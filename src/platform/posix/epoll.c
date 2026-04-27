#include <libc/epoll.h>
#include <libc/errno.h>
#include <abi/syscalls.h>
#include <stddef.h>

static inline long syscall_epoll_create1(int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_EPOLL_CREATE1),
          "D"(flags)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    long ret;
    register long r10 asm("r10") = (long)event;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_EPOLL_CTL),
          "D"(epfd),
          "S"(op),
          "d"(fd),
          "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    long ret;
    register long r10 asm("r10") = (long)timeout;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_EPOLL_WAIT),
          "D"(epfd),
          "S"(events),
          "d"(maxevents),
          "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const void *sigmask, size_t sigsetsize) {
    long ret;
    register long r10 asm("r10") = (long)timeout;
    register long r8  asm("r8")  = (long)sigmask;
    register long r9  asm("r9")  = (long)sigsetsize;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_EPOLL_PWAIT),
          "D"(epfd),
          "S"(events),
          "d"(maxevents),
          "r"(r10),
          "r"(r8),
          "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int epoll_create1(int flags) {
    long res = syscall_epoll_create1(flags);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return (int)res;
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    long res = syscall_epoll_ctl(epfd, op, fd, event);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return (int)res;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout) {
    long res = syscall_epoll_wait(epfd, events, maxevents, timeout);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return (int)res;
}

int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const void *sigmask) {
    size_t sigsetsize = 8; 
    
    long res = syscall_epoll_pwait(epfd, events, maxevents, timeout, sigmask, sigsetsize);
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return (int)res;
}