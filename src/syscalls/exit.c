#include <abi/syscalls.h>
#include <libc/syscalls/ioctl.h>
#include <stdint.h>
#include <libc/devices/console.h>

static inline long sys_exit(long n, long a1) {
    long ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void _exit(int code) {
    uint32_t restore_flags = TTY_ECHO | TTY_CANNONICAL;
    _ioctl(1, TTY_IOCTL_SET_FLAGS, &restore_flags);

    sys_exit(SYS_EXIT, code);
    __builtin_unreachable();
}
