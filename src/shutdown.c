#include <abi/syscalls.h>

static inline void syscall_shutdown() {
    __asm__ volatile (
        "int $0x80"
        :
        : "a"(SYS_SHUTDOWN)
        : "rcx", "r11", "memory"
    );
    __builtin_unreachable();
}

void _shutdown(){
    syscall_shutdown();
}