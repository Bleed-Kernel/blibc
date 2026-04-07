#include <abi/syscalls.h>

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

int _umount(const char *target)
{
    return (int)syscall_umount2(target, 0);
}

int _umount2(const char *target, int flags)
{
    return (int)syscall_umount2(target, flags);
}