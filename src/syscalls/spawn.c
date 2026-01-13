#include <abi/syscalls.h>

static inline int syscall_spawn(const char *path) {
    int pid;
    asm volatile(
        "int $0x80"
        : "=a"(pid)
        : "a"(SYS_SPAWN), "D"(path)
        : "rcx", "r11", "memory"
    );
    return pid;
}

int _spawn(const char *path){
    return syscall_spawn(path);
}