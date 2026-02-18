#include <abi/syscalls.h>
#include <libc/syscalls/stat.h>

static inline long syscall_stat(const char *path, struct stat *st) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_STAT),
          "D"(path),
          "S"(st)
        : "memory"
    );
    return ret;
}

long _stat(const char *path, struct stat *st) {
    return syscall_stat(path, st);
}
