#include <abi/syscalls.h>
#include <stdint.h>
#include <stddef.h>

static inline void* syscall_mmap(size_t pages) {
    void* ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_MMAP), "D"(pages)
        : "rcx", "r11", "memory"
    );
    if ((uintptr_t)ret == (uintptr_t)-1)
        return NULL;
    return ret;
}

void* _mmap(unsigned long pages) {
    return syscall_mmap(pages);
}
