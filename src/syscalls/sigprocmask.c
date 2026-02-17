#include <abi/syscalls.h>
#include <libc/syscalls/signals.h>

static inline long syscall_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    long ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_SIGPROCMASK),
          "D"(how),
          "S"(set),
          "d"(oldset)
        : "memory"
    );
    return ret;
}

long _sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    return syscall_sigprocmask(how, set, oldset);
}
