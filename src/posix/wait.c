#include <sys/wait.h>

#include <errno.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

pid_t waitpid(pid_t pid, int *status, int options) {
    long ret;
    int  st = 0;

    if (options & ~(WNOHANG | WUNTRACED | WCONTINUED)) {
        errno = EINVAL;
        return -1;
    }

    ret = _waitopts((long)pid, &st, options & WNOHANG);
    if (ret < 0) return (pid_t)posix_errno_from_ret(ret);
    if (ret > 0 && status)
        *status = st;

    return (pid_t)ret;
}

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}
