#include <abi/syscalls.h>

static inline long syscall_chdir(const char *path) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_CHDIR),
          "D"(path)
        : "memory"
    );
    return ret;
}

long _chdir(const char *path) {
    return syscall_chdir(path);
}