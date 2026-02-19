#include <stdint.h>
#include <abi/syscalls.h>

uint64_t syscall_taskcount(void) {
    uint64_t ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_TASKCOUNT)
        : "rcx", "r11", "memory"
    );
    return ret;
}

uint64_t _taskcount(void){
    return syscall_taskcount();
}