#include <abi/syscalls.h>

static inline long syscall_open(const char *path, int flags) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_OPEN),
          "D"(path),
          "S"(flags)
        : "memory"
    );
    return ret;
}

long _open(const char *path, int flags) {
    return syscall_open(path, flags);
}