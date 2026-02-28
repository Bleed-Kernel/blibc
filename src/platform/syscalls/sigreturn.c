#include <abi/syscalls.h>

long _sigreturn(void) {
    long ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_SIGRETURN)
        : "memory"
    );

    return ret;
}
