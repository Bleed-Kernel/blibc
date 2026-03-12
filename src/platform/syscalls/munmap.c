#include <abi/syscalls.h>


static inline void syscall_munmap(void* addr) {
    asm volatile(
        "syscall"
        :
        : "a"(SYS_MUNMAP), "D"(addr)
        : "rcx", "r11", "memory"
    );
}

void _munmap(void* addr) {
    syscall_munmap(addr);
}