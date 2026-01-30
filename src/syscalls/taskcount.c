#include <abi/syscalls.h>
#include <stdint.h>

static inline uint64_t syscall_taskcount(uint64_t *count) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_TASKCOUNT), "D"(count)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _taskcount(uint64_t *count) {
    if (syscall_taskcount(count) != 0)
        return 0;
    return -1;
}