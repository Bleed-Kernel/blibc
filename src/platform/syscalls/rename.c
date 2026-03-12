#include <abi/syscalls.h>

static inline int syscall_rename(const char *oldpath, const char *newpath) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_RENAME), "D"(oldpath), "S"(newpath)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _rename(const char *oldpath, const char *newpath) {
    return syscall_rename(oldpath, newpath);
}
