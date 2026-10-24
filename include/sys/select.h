#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include <sys/time.h>

#define FD_SETSIZE  1024

typedef struct {
    unsigned long bits[FD_SETSIZE / (8 * sizeof(unsigned long))];
} fd_set;

#define __FD_WORD(fd)   ((fd) / (8 * sizeof(unsigned long)))
#define __FD_BIT(fd)    (1UL << ((fd) % (8 * sizeof(unsigned long))))

#define FD_ZERO(set)     memset((set), 0, sizeof(fd_set))
#define FD_SET(fd, set)  ((set)->bits[__FD_WORD(fd)] |= __FD_BIT(fd))
#define FD_CLR(fd, set)  ((set)->bits[__FD_WORD(fd)] &= ~__FD_BIT(fd))
#define FD_ISSET(fd, set) (((set)->bits[__FD_WORD(fd)] & __FD_BIT(fd)) != 0)

// done with poll() underneath
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

#ifdef __cplusplus
}
#endif
