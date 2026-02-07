#include <abi/syscalls.h>
#include <stdint.h>

static inline uint64_t syscall_femtoseconds() {
    uint64_t ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_FEMTOSECONDS)
        : "memory"
    );
    return ret;
}

uint64_t _femtoseconds() {
    return syscall_femtoseconds();
}