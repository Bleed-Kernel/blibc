#include <libc/sys/stat.h>

#include <libc/syscalls/stat.h>

#include "compat_errno.h"

// the kernel is yet to make use of a full stat but i do intend on it soon just like adding
// vfs stuff for making files from userspace this is basically a stub for comptablilty for now

int stat(const char *path, struct stat *st) {
    long ret = _stat(path, st);
    if (ret < 0) {
        return (int)posix_errno_from_ret(ret);
    }
    return 0;
}
