#include <abi/syscalls.h>
#include <libc/scheduler.h>

static inline int syscall_taskinfo(uint64_t pid, user_task_info_t *buf)
{
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_TASKINFO), "D"(pid), "S"(buf)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _taskinfo(uint64_t pid, user_task_info_t *buf) {
    return syscall_taskinfo(pid, buf);
}
