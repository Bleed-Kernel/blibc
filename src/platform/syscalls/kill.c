#include <abi/syscalls.h>

long _kill(long pid, long sig) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_KILL),
          "D"(pid),
          "S"(sig)
        : "rcx", "r11", "memory"
    );

    return ret;
}
