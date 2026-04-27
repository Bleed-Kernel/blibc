#include <abi/syscalls.h>

static inline void syscall_yield() {
    __asm__ volatile (
        "syscall"
        :
        : "a"(SYS_YIELD)
        : "rcx", "r11", "memory"
    );
}

void _yeild(){
    syscall_yield();
}