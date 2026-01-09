#include <abi/syscalls.h>

static inline long syscall_waitpid(long pid){
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_WAITPID), "D"(pid)
        : "memory"
    );

    return ret;
}

long _waitpid(long pid){
    return syscall_waitpid(pid);
}