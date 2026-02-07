#include <abi/syscalls.h>

static inline long syscall_ioctl(int fd, unsigned long request, void* arg) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_IOCTL),
          "D"(fd),
          "S"(request),
          "d"(arg)
        : "memory"
    );
    return ret;
}

int _ioctl(int fd, unsigned long request, void* arg) {
    return (int)syscall_ioctl(fd, request, arg);
}