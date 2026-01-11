#include <abi/syscalls.h>

void _tkill(long pid){
    asm volatile(
        "int $0x80"
        : 
        : "a"(SYS_TKILL), "D"(pid)
        : "memory"
    );
}