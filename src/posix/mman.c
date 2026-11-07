#include <sys/mman.h>
#include <errno.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    long ret = _mmap_full(addr, len, prot, flags, fd, offset);
    if (ret < 0 && ret > -4096) {
        errno = (int)-ret;
        return MAP_FAILED;
    }
    return (void *)ret;
}

int munmap(void *addr, size_t len) {
    // the kernel takes 0 as the whole mapping, posix says thats an error
    if (!len) {
        errno = EINVAL;
        return -1;
    }
    long ret = _munmap_range(addr, len);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return 0;
}

int mprotect(void *addr, size_t len, int prot) {
    long ret = _mprotect(addr, len, prot);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return 0;
}

int madvise(void *addr, size_t len, int advice) {
    long ret = _madvise(addr, len, advice);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return 0;
}

int posix_madvise(void *addr, size_t len, int advice) {
    // POSIX_MADV_DONTNEED is only a hint, it mustnt throw the data away like MADV_DONTNEED
    if (advice == POSIX_MADV_DONTNEED)
        return 0;
    long ret = _madvise(addr, len, advice);
    return ret < 0 ? (int)-ret : 0;
}

// nothing is backed by files yet and nothing gets swapped out
int msync(void *addr, size_t len, int flags) {
    (void)addr;
    (void)len;
    (void)flags;
    return 0;
}

int mlock(const void *addr, size_t len) {
    (void)addr;
    (void)len;
    return 0;
}

int munlock(const void *addr, size_t len) {
    (void)addr;
    (void)len;
    return 0;
}

int mlockall(int flags) {
    (void)flags;
    return 0;
}

int munlockall(void) {
    return 0;
}

static long mman_syscall2(long n, long a, long b) {
    long ret;
    asm volatile("syscall" : "=a"(ret) : "a"(n), "D"(a), "S"(b) : "rcx", "r11", "memory");
    return ret;
}

int memfd_create(const char *name, unsigned int flags) {
    long ret = mman_syscall2(319, (long)name, flags);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int ftruncate(int fd, off_t length) {
    long ret = mman_syscall2(77, fd, length);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return 0;
}

int shm_open(const char *name, int flags, mode_t mode) {
    (void)name;
    (void)flags;
    (void)mode;
    errno = ENOSYS;
    return -1;
}

int shm_unlink(const char *name) {
    (void)name;
    errno = ENOSYS;
    return -1;
}
