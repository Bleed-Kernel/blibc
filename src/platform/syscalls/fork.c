#include <abi/syscalls.h>

long _fork(void) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_FORK)
        : "memory"
    );
    return ret;
}
