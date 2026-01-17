#include <abi/syscalls.h>
#include <libc/syscalls/readdir.h>
#include <stddef.h>
#include <stdint.h>

static inline long syscall_readdir(int fd, size_t index, dirent_t *out) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_READDIR),
          "D"(fd),
          "S"(index),
          "d"(out)
        : "memory"
    );
    return ret;
}

long _readdir(int fd, size_t index, dirent_t *out) {
    if (!out) return -1;
    return syscall_readdir(fd, index, out);
}