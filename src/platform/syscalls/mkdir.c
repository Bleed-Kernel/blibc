#include <abi/syscalls.h>

static inline int syscall_mkdir(const char *path, int mode) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MKDIR), "D"(path), "S"(mode)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _mkdir(const char *path, int mode) {
    return syscall_mkdir(path, mode);
}
