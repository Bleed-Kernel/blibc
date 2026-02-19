#include <abi/syscalls.h>
#include <libc/syscalls/time.h>

static inline int syscall_time(time_t *buf) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_TIME), "D"(buf)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _time(time_t *buf) {
    return syscall_time(buf);
}