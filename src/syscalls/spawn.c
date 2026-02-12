#include <abi/syscalls.h>
#include <libc/stddef.h>
#include <libc/stdint.h>

static inline int syscall_spawn(const char *path, const char *const argv[], uint64_t argc) {
    int pid;
    asm volatile(
        "int $0x80"
        : "=a"(pid)
        : "a"(SYS_SPAWN), "D"(path), "S"(argv), "d"(argc)
        : "rcx", "r11", "memory"
    );
    return pid;
}

int _spawn(const char *path, const char *const argv[], uint64_t argc) {
    return syscall_spawn(path, argv, argc);
}
