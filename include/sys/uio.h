#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <sys/types.h>

#define IOV_MAX 1024

struct iovec {
    void  *iov_base;
    size_t iov_len;
};

ssize_t readv(int fd, const struct iovec *iov, int count);
ssize_t writev(int fd, const struct iovec *iov, int count);
ssize_t preadv(int fd, const struct iovec *iov, int count, off_t offset);
ssize_t pwritev(int fd, const struct iovec *iov, int count, off_t offset);

#ifdef __cplusplus
}
#endif
