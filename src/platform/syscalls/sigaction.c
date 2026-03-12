#include <abi/syscalls.h>
#include <libc/syscalls/signals.h>

static inline long syscall_sigaction(int sig, const sigaction_t *act, sigaction_t *oldact) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SIGACTION),
          "D"(sig),
          "S"(act),
          "d"(oldact)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _sigaction(int sig, const sigaction_t *act, sigaction_t *oldact) {
    return syscall_sigaction(sig, act, oldact);
}
