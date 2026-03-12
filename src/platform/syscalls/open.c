#include <abi/syscalls.h>

static inline long syscall_open(const char *path, int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_OPEN),
          "D"(path),
          "S"(flags)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _open(const char *path, int flags) {
    return syscall_open(path, flags);
}