#include <abi/syscalls.h>

#define PAGE_SIZE 4096

static inline long sys_free(long n, void* addr, long pages) {
    long ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "D"(addr), "S"(pages)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void free(void* ptr, long bytes) {
    if (!ptr || bytes <= 0)
        return;

    sys_free(SYS_FREE, ptr, (bytes + (PAGE_SIZE - 1)) / PAGE_SIZE);
}