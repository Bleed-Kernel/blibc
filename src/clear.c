#include <abi/syscalls.h>

static inline void syscall_clear(int fd) {
    __asm__ volatile (
        "int $0x80"
        :
        : "a"(SYS_CLEAR),
          "D"(fd)
        : "rcx", "r11", "memory"
    );
}

void _clear(int fd){
    syscall_clear(fd);
}