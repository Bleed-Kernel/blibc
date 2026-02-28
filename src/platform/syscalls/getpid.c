#include <abi/syscalls.h>

long _getpid(void) {
    long ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_GETPID)
        : "memory"
    );

    return ret;
}
