#include <abi/syscalls.h>
#include <stddef.h>
#include <stdint.h>

static inline void* syscall_mapfb(size_t *out_pages) {
    void *ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MAPFB),
          "D"(out_pages)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void* mapfb(size_t *out_pages) {
    return syscall_mapfb(out_pages);
}