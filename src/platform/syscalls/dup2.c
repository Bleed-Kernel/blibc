#include <abi/syscalls.h>

static inline long syscall_dup2(int oldfd, int newfd) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_DUP2), "D"(oldfd), "S"(newfd)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _dup2(int oldfd, int newfd) {
    return syscall_dup2(oldfd, newfd);
}
