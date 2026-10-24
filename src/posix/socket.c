#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// no network stack in the kernel yet, so sockets fail like they would on a machine without one.

const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
int h_errno;

static int no_net(void) {
    errno = ENOSYS;
    return -1;
}

int socket(int domain, int type, int protocol) { (void)domain; (void)type; (void)protocol; return no_net(); }
int bind(int fd, const struct sockaddr *addr, socklen_t len) { (void)fd; (void)addr; (void)len; return no_net(); }
int listen(int fd, int backlog) { (void)fd; (void)backlog; return no_net(); }
int accept(int fd, struct sockaddr *addr, socklen_t *len) { (void)fd; (void)addr; (void)len; return no_net(); }
int connect(int fd, const struct sockaddr *addr, socklen_t len) { (void)fd; (void)addr; (void)len; return no_net(); }
int setsockopt(int fd, int level, int name, const void *val, socklen_t len) { (void)fd; (void)level; (void)name; (void)val; (void)len; return no_net(); }
int getsockopt(int fd, int level, int name, void *val, socklen_t *len) { (void)fd; (void)level; (void)name; (void)val; (void)len; return no_net(); }
int getsockname(int fd, struct sockaddr *addr, socklen_t *len) { (void)fd; (void)addr; (void)len; return no_net(); }
int getpeername(int fd, struct sockaddr *addr, socklen_t *len) { (void)fd; (void)addr; (void)len; return no_net(); }
int shutdown(int fd, int how) { (void)fd; (void)how; return no_net(); }

ssize_t send(int fd, const void *buf, size_t len, int flags) {
    (void)fd; (void)buf; (void)len; (void)flags;
    return no_net();
}

ssize_t recv(int fd, void *buf, size_t len, int flags) {
    (void)fd; (void)buf; (void)len; (void)flags;
    return no_net();
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen) {
    (void)fd; (void)buf; (void)len; (void)flags; (void)addr; (void)addrlen;
    return no_net();
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen) {
    (void)fd; (void)buf; (void)len; (void)flags; (void)addr; (void)addrlen;
    return no_net();
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

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    (void)node; (void)service; (void)hints;
    if (res)
        *res = NULL;
    return EAI_NONAME;
}

void freeaddrinfo(struct addrinfo *res) {
    while (res) {
        struct addrinfo *next = res->ai_next;
        free(res);
        res = next;
    }
}

const char *gai_strerror(int err) {
    switch (err) {
    case EAI_NONAME:   return "Name or service not known";
    case EAI_AGAIN:    return "Temporary failure in name resolution";
    case EAI_FAIL:     return "Non-recoverable failure in name resolution";
    case EAI_FAMILY:   return "Address family not supported";
    case EAI_MEMORY:   return "Memory allocation failure";
    default:           return "Unknown error";
    }
}

struct hostent *gethostbyname(const char *name) {
    (void)name;
    h_errno = HOST_NOT_FOUND;
    return NULL;
}
