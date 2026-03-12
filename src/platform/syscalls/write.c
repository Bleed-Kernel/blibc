#include <abi/syscalls.h>

static inline long sys_write(long n, long a1, long a2, long a3) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _write(int fd, const void *buf, long len) {
    return sys_write(SYS_WRITE, fd, (long)buf, len);
}
