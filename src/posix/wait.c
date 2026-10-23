#include <sys/wait.h>

#include <errno.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

pid_t waitpid(pid_t pid, int *status, int options) {
    long ret;

    if (options != 0) {
        errno = EINVAL;
        return -1;
    }

    ret = _waitstatus((long)pid, status);
    if (ret < 0) return (pid_t)posix_errno_from_ret(ret);

    return (pid_t)ret;
}

pid_t wait(int *status) {
    errno = ENOSYS;
    (void)status;
    return -1;
}
