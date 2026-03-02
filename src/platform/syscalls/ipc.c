#include <abi/syscalls.h>
#include <libc/syscalls/ipc.h>

long _ipc_send(uint64_t target_pid, uint64_t src_addr, uint64_t pages) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_IPC_SEND), "D"(target_pid), "S"(src_addr), "d"(pages)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _ipc_recv(ipc_message_t *msg) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_IPC_RECV), "D"(msg)
        : "rcx", "r11", "memory"
    );
    return ret;
}
