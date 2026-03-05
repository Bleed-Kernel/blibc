#include <libc/poll.h>

#include <libc/errno.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    (void)fds;
    (void)nfds;
    (void)timeout;
    errno = ENOSYS;
    return -1;
}
