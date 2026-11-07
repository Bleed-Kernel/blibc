#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>

#define PROT_NONE       0x0
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4

#define MAP_SHARED      0x01
#define MAP_PRIVATE     0x02
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

#define MREMAP_MAYMOVE  1
#define MREMAP_FIXED    2
#define MAP_SHARED_VALIDATE 0x03
#define MAP_TYPE        0x0f
#define MAP_GROWSDOWN   0x0100
#define MAP_DENYWRITE   0x0800
#define MAP_EXECUTABLE  0x1000
#define MAP_LOCKED      0x2000
#define MAP_NORESERVE   0x4000
#define MAP_POPULATE    0x8000
#define MAP_NONBLOCK    0x10000
#define MAP_STACK       0x20000
#define MAP_HUGETLB     0x40000
#define MAP_FIXED_NOREPLACE 0x100000

#define MADV_NORMAL     0
#define MADV_RANDOM     1
#define MADV_SEQUENTIAL 2
#define MADV_WILLNEED   3
#define MADV_DONTNEED   4
#define MADV_FREE       8
#define MADV_DONTFORK   10
#define MADV_DOFORK     11
#define MADV_HUGEPAGE   14
#define MADV_NOHUGEPAGE 15
#define MADV_DONTDUMP   16

#define POSIX_MADV_NORMAL     0
#define POSIX_MADV_RANDOM     1
#define POSIX_MADV_SEQUENTIAL 2
#define POSIX_MADV_WILLNEED   3
#define POSIX_MADV_DONTNEED   4

#define MS_ASYNC        1
#define MS_INVALIDATE   2
#define MS_SYNC         4

#define MCL_CURRENT     1
#define MCL_FUTURE      2

#define MAP_FAILED      ((void *)-1)

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t len);
int mprotect(void *addr, size_t len, int prot);
int madvise(void *addr, size_t len, int advice);

#define MFD_CLOEXEC       0x1
#define MFD_ALLOW_SEALING 0x2

// an anonymous file in memory, mmap MAP_SHARED of it is how processes share pages
int memfd_create(const char *name, unsigned int flags);

// no named shared memory yet, these give ENOSYS anyway
int shm_open(const char *name, int flags, mode_t mode);
int shm_unlink(const char *name);
int posix_madvise(void *addr, size_t len, int advice);
int msync(void *addr, size_t len, int flags);
int mlock(const void *addr, size_t len);
int munlock(const void *addr, size_t len);
int mlockall(int flags);
int munlockall(void);

// only shrinking in place, growing says ENOMEM and callers (sqlite) fall back to a new map
void *mremap(void *old, size_t old_len, size_t new_len, int flags, ...);

#ifdef __cplusplus
}
#endif
