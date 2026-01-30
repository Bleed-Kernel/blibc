#include <abi/syscalls.h>
#include <libc/sysinfo/sysinfo.h>

static inline int syscall_meminfo(system_memory_info_t *buf) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_MEMINFO), "D"(buf)
        : "rcx", "r11", "memory"
    );
    return ret;
}