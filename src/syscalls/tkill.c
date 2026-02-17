#include <abi/syscalls.h>

long _tkill(long pid, long sig) {
    long ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_TKILL),
          "D"(pid),
          "S"(sig)
        : "memory"
    );

    return ret;
}