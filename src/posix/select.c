#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    if (nfds < 0 || nfds > FD_SETSIZE) {
        errno = EINVAL;
        return -1;
    }

    struct pollfd fds[FD_SETSIZE];
    int count = 0;

    for (int fd = 0; fd < nfds; fd++) {
        short ev = 0;
        if (readfds && FD_ISSET(fd, readfds))
            ev |= POLLIN;
        if (writefds && FD_ISSET(fd, writefds))
            ev |= POLLOUT;
        if (exceptfds && FD_ISSET(fd, exceptfds))
            ev |= POLLPRI;
        if (!ev)
            continue;
        fds[count].fd      = fd;
        fds[count].events  = ev;
        fds[count].revents = 0;
        count++;
    }

    int ms = -1;
    if (timeout)
        ms = (int)(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);

    int ret = poll(fds, (nfds_t)count, ms);
    if (ret < 0)
        return -1;

    if (readfds)
        FD_ZERO(readfds);
    if (writefds)
        FD_ZERO(writefds);
    if (exceptfds)
        FD_ZERO(exceptfds);

    // select counts bits, not fds
    int ready = 0;
    for (int i = 0; i < count; i++) {
        short re = fds[i].revents;
        if (readfds && (re & (POLLIN | POLLHUP | POLLERR))) {
            FD_SET(fds[i].fd, readfds);
            ready++;
        }
        if (writefds && (re & (POLLOUT | POLLERR))) {
            FD_SET(fds[i].fd, writefds);
            ready++;
        }
        if (exceptfds && (re & POLLPRI)) {
            FD_SET(fds[i].fd, exceptfds);
            ready++;
        }
    }
    return ready;
}
