#include <poll.h>
#include <bleed/syscalls.h>
#include "compat_errno.h"

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    long ret = _poll(fds, nfds, timeout);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}
