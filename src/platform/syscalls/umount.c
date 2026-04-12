#include <abi/syscalls.h>
#include <errno.h>

static inline long syscall_umount2(const char *target, int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_UMOUNT),
          "D"(target),
          "S"(flags)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _umount(const char *target) {
    long r = syscall_umount2(target, 0);
    if (r < 0) {
        errno = (int)-r;
        return -1;
    }
    return 0;
}

int _umount2(const char *target, int flags) {
    long r = syscall_umount2(target, flags);
    if (r < 0) {
        errno = (int)-r;
        return -1;
    }
    return 0;
}