#include <abi/syscalls.h>

static inline long syscall_chdir(const char *path) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_CHDIR),
          "D"(path)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _chdir(const char *path) {
    return syscall_chdir(path);
}