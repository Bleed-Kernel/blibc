#include <abi/syscalls.h>
#include <stddef.h>

static inline long syscall_getcwd(char *buf, long size) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_GETCWD),
          "D"(buf),
          "S"(size)
        : "rcx", "r11", "memory"
    );
    return ret;
}

char *_getcwd(char *buf, long size) {
    if (!buf) return NULL;
    long r = syscall_getcwd(buf, size);
    if (r < 0) return NULL;
    return buf;
}