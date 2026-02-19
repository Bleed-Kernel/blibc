#include <abi/syscalls.h>


static inline void syscall_munmap(void* addr) {
    asm volatile(
        "int $0x80"
        :
        : "a"(SYS_MUNMAP), "D"(addr)
        : "rcx", "r11", "memory"
    );
}

void _munmap(void* addr) {
    syscall_munmap(addr);
}