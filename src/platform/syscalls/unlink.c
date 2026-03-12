#include <abi/syscalls.h>

static inline int syscall_unlink(const char *path) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_UNLINK), "D"(path)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _unlink(const char *path) {
    return syscall_unlink(path);
}
