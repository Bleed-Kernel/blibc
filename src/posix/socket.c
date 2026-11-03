#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <bleed/syscalls.h>
#include "compat_errno.h"

const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;

static long net_syscall(long nr, long a, long b, long c, long d, long e, long f) {
    long ret;
    register long r10 asm("r10") = d;
    register long r8  asm("r8")  = e;
    register long r9  asm("r9")  = f;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(nr), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static long net_ret(long ret) {
    return ret < 0 ? posix_errno_from_ret(ret) : ret;
}

int socket(int domain, int type, int protocol) {
    return (int)net_ret(net_syscall(SYS_SOCKET, domain, type, protocol, 0, 0, 0));
}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
    return (int)net_ret(net_syscall(SYS_BIND, fd, (long)addr, len, 0, 0, 0));
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
    return (int)net_ret(net_syscall(SYS_CONNECT, fd, (long)addr, len, 0, 0, 0));
}

int setsockopt(int fd, int level, int name, const void *val, socklen_t len) {
    return (int)net_ret(net_syscall(SYS_SETSOCKOPT, fd, level, name, (long)val, len, 0));
}

int getsockopt(int fd, int level, int name, void *val, socklen_t *len) {
    return (int)net_ret(net_syscall(SYS_GETSOCKOPT, fd, level, name, (long)val, (long)len, 0));
}

int getsockname(int fd, struct sockaddr *addr, socklen_t *len) {
    return (int)net_ret(net_syscall(SYS_GETSOCKNAME, fd, (long)addr, (long)len, 0, 0, 0));
}

int getpeername(int fd, struct sockaddr *addr, socklen_t *len) {
    return (int)net_ret(net_syscall(SYS_GETPEERNAME, fd, (long)addr, (long)len, 0, 0, 0));
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen) {
    return net_ret(net_syscall(SYS_SENDTO, fd, (long)buf, (long)len, flags, (long)addr, addrlen));
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen) {
    return net_ret(net_syscall(SYS_RECVFROM, fd, (long)buf, (long)len, flags, (long)addr, (long)addrlen));
}

ssize_t send(int fd, const void *buf, size_t len, int flags) {
    return sendto(fd, buf, len, flags, NULL, 0);
}

ssize_t recv(int fd, void *buf, size_t len, int flags) {
    return recvfrom(fd, buf, len, flags, NULL, NULL);
}

int listen(int fd, int backlog) {
    return (int)net_ret(net_syscall(SYS_LISTEN, fd, backlog, 0, 0, 0, 0));
}

int accept(int fd, struct sockaddr *addr, socklen_t *len) {
    return (int)net_ret(net_syscall(SYS_ACCEPT, fd, (long)addr, (long)len, 0, 0, 0));
}

int accept4(int fd, struct sockaddr *addr, socklen_t *len, int flags) {
    return (int)net_ret(net_syscall(SYS_ACCEPT4, fd, (long)addr, (long)len, flags, 0, 0));
}

int shutdown(int fd, int how) {
    return (int)net_ret(net_syscall(SYS_SOCK_SHUTDOWN, fd, how, 0, 0, 0, 0));
}

static size_t iov_total(const struct iovec *iov, size_t count) {
    size_t total = 0;
    for (size_t i = 0; i < count; i++)
        total += iov[i].iov_len;
    return total;
}

// the kernel takes one buffer, a datagram has to stay in one piece
ssize_t sendmsg(int fd, const struct msghdr *msg, int flags) {
    size_t total = iov_total(msg->msg_iov, msg->msg_iovlen);
    char  *flat  = malloc(total ? total : 1);
    if (!flat) {
        errno = ENOMEM;
        return -1;
    }

    size_t at = 0;
    for (size_t i = 0; i < msg->msg_iovlen; i++) {
        memcpy(flat + at, msg->msg_iov[i].iov_base, msg->msg_iov[i].iov_len);
        at += msg->msg_iov[i].iov_len;
    }

    ssize_t r = sendto(fd, flat, total, flags, msg->msg_name, msg->msg_namelen);
    free(flat);
    return r;
}

ssize_t recvmsg(int fd, struct msghdr *msg, int flags) {
    size_t total = iov_total(msg->msg_iov, msg->msg_iovlen);
    char  *flat  = malloc(total ? total : 1);
    if (!flat) {
        errno = ENOMEM;
        return -1;
    }

    ssize_t r = recvfrom(fd, flat, total, flags, msg->msg_name, msg->msg_name ? &msg->msg_namelen : NULL);
    if (r > 0) {
        size_t at = 0;
        for (size_t i = 0; i < msg->msg_iovlen && at < (size_t)r; i++) {
            size_t n = (size_t)r - at;
            if (n > msg->msg_iov[i].iov_len)
                n = msg->msg_iov[i].iov_len;
            memcpy(msg->msg_iov[i].iov_base, flat + at, n);
            at += n;
        }
    }
    msg->msg_controllen = 0;
    msg->msg_flags      = 0;
    free(flat);
    return r;
}

ssize_t readv(int fd, const struct iovec *iov, int count) {
    ssize_t total = 0;
    for (int i = 0; i < count; i++) {
        if (!iov[i].iov_len)
            continue;
        ssize_t r = read(fd, iov[i].iov_base, iov[i].iov_len);
        if (r < 0)
            return total ? total : r;
        total += r;
        if ((size_t)r < iov[i].iov_len)
            break;
    }
    return total;
}

ssize_t writev(int fd, const struct iovec *iov, int count) {
    ssize_t total = 0;
    for (int i = 0; i < count; i++) {
        if (!iov[i].iov_len)
            continue;
        ssize_t r = write(fd, iov[i].iov_base, iov[i].iov_len);
        if (r < 0)
            return total ? total : r;
        total += r;
        if ((size_t)r < iov[i].iov_len)
            break;
    }
    return total;
}

// a.b.c.d only, each part 0-255
int inet_aton(const char *cp, struct in_addr *addr) {
    uint32_t ip = 0;
    for (int part = 0; part < 4; part++) {
        if (*cp < '0' || *cp > '9')
            return 0;
        unsigned v = 0;
        while (*cp >= '0' && *cp <= '9') {
            v = v * 10 + (unsigned)(*cp++ - '0');
            if (v > 255)
                return 0;
        }
        ip = (ip << 8) | v;
        if (part < 3 && *cp++ != '.')
            return 0;
    }
    if (*cp)
        return 0;
    if (addr)
        addr->s_addr = htonl(ip);
    return 1;
}

in_addr_t inet_addr(const char *cp) {
    struct in_addr a;
    return inet_aton(cp, &a) ? a.s_addr : INADDR_NONE;
}

char *inet_ntoa(struct in_addr in) {
    static char buf[INET_ADDRSTRLEN];
    const uint8_t *b = (const uint8_t *)&in.s_addr;
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
    return buf;
}

int inet_pton(int af, const char *src, void *dst) {
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    return inet_aton(src, (struct in_addr *)dst);
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return NULL;
    }
    const uint8_t *b = (const uint8_t *)src;
    if (snprintf(dst, size, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]) >= (int)size) {
        errno = ENOSPC;
        return NULL;
    }
    return dst;
}
