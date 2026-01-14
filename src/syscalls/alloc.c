#include <abi/syscalls.h>

#define PAGE_SIZE 4096

static inline void* sys_alloc(long n, long pages) {
    void* ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "D"(pages)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void* malloc(long bytes) {
    return sys_alloc(SYS_ALLOC, (bytes + (PAGE_SIZE - 1)) / PAGE_SIZE);
}