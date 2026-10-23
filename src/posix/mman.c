#include <sys/mman.h>
#include <errno.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

#define PAGE_SIZE 4096UL

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    (void)addr;
    (void)offset;

    if (!len || (flags & MAP_FIXED)) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    if (!(flags & MAP_ANONYMOUS) || fd != -1) {
        errno = ENODEV;
        return MAP_FAILED;
    }

    void *p = _mmap((len + PAGE_SIZE - 1) / PAGE_SIZE);
    if (!p || (long)p < 0) {
        errno = ENOMEM;
        return MAP_FAILED;
    }

    // fresh mappings come back read write no exec, anything else needs a mprotect
    if (prot != (PROT_READ | PROT_WRITE) && mprotect(p, len, prot) < 0) {
        _munmap(p);
        return MAP_FAILED;
    }
    return p;
}

int munmap(void *addr, size_t len) {
    (void)len;
    _munmap(addr);
    return 0;
}

int mprotect(void *addr, size_t len, int prot) {
    long ret = _mprotect(addr, len, prot);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return 0;
}
