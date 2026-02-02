#include <abi/syscalls.h>
#include <libc/scheduler.h>
#include <stdint.h>

uint64_t syscall_taskinfo(uint64_t pid, user_task_info_t *info) {
    uint64_t ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_TASKINFO), "D"(pid), "S"(info)
        : "rcx", "r11", "memory"
    );
    return ret;
}

uint64_t _taskinfo(uint64_t pid, user_task_info_t *info){
    return syscall_taskinfo(pid, info);
}