#include <abi/syscalls.h>

static inline int syscall_close(int fd) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_CLOSE), "D"(fd)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _close(int fd) {
    return syscall_close(fd);
}