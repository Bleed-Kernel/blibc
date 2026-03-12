#include <abi/syscalls.h>

long _getpid(void) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_GETPID)
        : "rcx", "r11", "memory"
    );

    return ret;
}
