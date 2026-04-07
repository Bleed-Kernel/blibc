#include <abi/syscalls.h>

static inline long syscall_mount(const char *source,
                                 const char *target,
                                 const char *fstype,
                                 unsigned long flags,
                                 const void *data)
{
    long ret;

    register unsigned long r10 asm("r10") = flags;
    register const void *r8 asm("r8") = data;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MOUNT),
          "D"(source),
          "S"(target),
          "d"(fstype),
          "r"(r10),
          "r"(r8)
        : "rcx", "r11", "memory"
    );

    return ret;
}

int _mount(const char *source,
           const char *target,
           const char *fstype,
           unsigned long flags,
           const void *data)
{
    return (int)syscall_mount(source, target, fstype, flags, data);
}
