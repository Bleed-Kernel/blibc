#pragma once

#include <libc/errno.h>

static inline long posix_errno_from_ret(long ret) {
    long code = -ret;

    if (code <= 0 || code > 4095) {
        errno = EIO;
    } else {
        errno = (int)code;
    }
    return -1;
}
