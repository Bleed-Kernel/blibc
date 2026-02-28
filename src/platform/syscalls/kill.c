#include <abi/syscalls.h>

long _kill(long pid, long sig) {
    long ret;

    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_KILL),
          "D"(pid),
          "S"(sig)
        : "memory"
    );

    return ret;
}
