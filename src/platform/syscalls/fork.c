#include <abi/syscalls.h>

long _fork(void) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_FORK)
        : "rcx", "r11", "memory"
    );
    return ret;
}
