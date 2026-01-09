#include <abi/syscalls.h>

static inline void syscall_reboot() {
    __asm__ volatile (
        "int $0x80"
        :
        : "a"(SYS_REBOOT)
        : "rcx", "r11", "memory"
    );
    __builtin_unreachable();
}

void _reboot(){
    syscall_reboot();
}