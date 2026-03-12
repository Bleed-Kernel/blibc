#include <abi/syscalls.h>

long _sigreturn(void) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SIGRETURN)
        : "rcx", "r11", "memory"
    );

    return ret;
}
