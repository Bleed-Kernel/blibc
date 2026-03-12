#include <abi/syscalls.h>

static inline long syscall_waitpid(long pid){
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_WAITPID), "D"(pid)
        : "rcx", "r11", "memory"
    );

    return ret;
}

long _waitpid(long pid){
    return syscall_waitpid(pid);
}