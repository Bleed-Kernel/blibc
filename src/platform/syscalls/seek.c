#include <abi/syscalls.h>

static inline long syscall_seek(int fd, long offset, int whence) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_SEEK),
          "D"(fd),
          "S"(offset),
          "d"(whence)
        : "memory"
    );
    return ret;
}

long _seek(int fd, long offset, int whence) {
    return syscall_seek(fd, offset, whence);
}