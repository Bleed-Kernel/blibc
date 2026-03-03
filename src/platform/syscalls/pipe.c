#include <abi/syscalls.h>

static inline long syscall_pipe(int fds[2]) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_PIPE), "D"(fds)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _pipe(int fds[2]) {
    return syscall_pipe(fds);
}
