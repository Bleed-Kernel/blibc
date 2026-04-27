#pragma once

#include <stdint.h>
#include <stddef.h>

// event flag bits
#define EPOLLIN        (1u << 0)
#define EPOLLPRI       (1u << 1)
#define EPOLLOUT       (1u << 2)
#define EPOLLERR       (1u << 3)
#define EPOLLHUP       (1u << 4)
#define EPOLLRDNORM    (1u << 6)
#define EPOLLRDBAND    (1u << 7)
#define EPOLLWRNORM    (1u << 8)
#define EPOLLWRBAND    (1u << 9)
#define EPOLLMSG       (1u << 10)
#define EPOLLRDHUP     (1u << 13)
#define EPOLLEXCLUSIVE (1u << 28)
#define EPOLLWAKEUP    (1u << 29)
#define EPOLLONESHOT   (1u << 30)
#define EPOLLET        (1u << 31)

// epoll control opcodes
#define EPOLL_CTL_ADD   1
#define EPOLL_CTL_DEL   2
#define EPOLL_CTL_MOD   3
#define EPOLL_CLOEXEC   (1 << 0)

typedef union epoll_data {
    void       *ptr;
    int         fd;
    uint32_t    u32;
    uint64_t    u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;
    epoll_data_t data;
} __attribute__((packed));

typedef struct epoll_event epoll_event_t;

int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const void *sigmask);
