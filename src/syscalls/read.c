#include <abi/syscalls.h>

static inline long syscall_read(int fd, void *buf, unsigned long len) {
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_READ),
          "D"(fd),
          "S"(buf),
          "d"(len)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _read(int fd, void *buf, unsigned long len){
    return syscall_read(fd, buf, len);
}