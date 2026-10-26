#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

// udp over ipv4 only for now, SOCK_STREAM and AF_INET6 sockets fail

typedef unsigned short sa_family_t;
typedef unsigned int   socklen_t;

struct sockaddr {
    sa_family_t sa_family;
    char        sa_data[14];
};

struct sockaddr_storage {
    sa_family_t ss_family;
    char        ss_pad1[6];
    uint64_t    ss_align;
    char        ss_pad2[112];
};

#define AF_UNSPEC       0
#define AF_UNIX         1
#define AF_INET         2
#define AF_INET6        10
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6

#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
#define SOCK_NONBLOCK   04000
#define SOCK_CLOEXEC    02000000

#define SOL_SOCKET      1
#define SO_REUSEADDR    2
#define SO_TYPE         3
#define SO_ERROR        4
#define SO_BROADCAST    6
#define SO_SNDBUF       7
#define SO_RCVBUF       8
#define SO_KEEPALIVE    9
#define SO_RCVTIMEO     20
#define SO_SNDTIMEO     21

#define MSG_PEEK        0x2
#define MSG_DONTWAIT    0x40
#define MSG_NOSIGNAL    0x4000

#define SHUT_RD         0
#define SHUT_WR         1
#define SHUT_RDWR       2

int socket(int domain, int type, int protocol);
int bind(int fd, const struct sockaddr *addr, socklen_t len);
int listen(int fd, int backlog);
int accept(int fd, struct sockaddr *addr, socklen_t *len);
int connect(int fd, const struct sockaddr *addr, socklen_t len);
ssize_t send(int fd, const void *buf, size_t len, int flags);
ssize_t recv(int fd, void *buf, size_t len, int flags);
ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen);
ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen);
int setsockopt(int fd, int level, int name, const void *val, socklen_t len);
int getsockopt(int fd, int level, int name, void *val, socklen_t *len);
int getsockname(int fd, struct sockaddr *addr, socklen_t *len);
int getpeername(int fd, struct sockaddr *addr, socklen_t *len);
int shutdown(int fd, int how);

#ifdef __cplusplus
}
#endif
