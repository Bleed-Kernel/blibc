#include <abi/syscalls.h>

static inline void syscall_yield() {
    __asm__ volatile (
        "int $0x80"
        :
        : "a"(SYS_YEILD)
        : "rcx", "r11", "memory"
    );
}

void _yeild(){
    syscall_yield();
}