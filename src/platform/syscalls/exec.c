#include <abi/syscalls.h>
#include <libc/stdint.h>

long _exec(const char *path, const char *const argv[], uint64_t argc) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_EXEC), "D"(path), "S"(argv), "d"(argc)
        : "rcx", "r11", "memory"
    );
    return ret;
}
