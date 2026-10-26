#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <bleed/syscalls.h>
#include <bleed/netdevice.h>

// A records over udp to the dns server dhcp gave the kernel, no cache tho

#define DNS_PORT        53
#define DNS_TRIES       2
#define DNS_WAIT_MS     1500
#define DNS_MAX         512     // a plain udp answer never goes past this
#define DNS_TYPE_A      1
#define DNS_CLASS_IN    1
#define DNS_FLAG_RD     0x0100
#define DNS_RCODE_MASK  0x000F

int h_errno;

static int net_info(struct net_device_info *info) {
    int fd = _open("/dev/net", O_RDONLY);
    if (fd < 0)
        return -1;
    long r = _ioctl(fd, NET_DEVICE_INFO, info);
    _close(fd);
    return r == 0 ? 0 : -1;
}

static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }

static const uint8_t *dns_skip_name(const uint8_t *p, const uint8_t *end) {
    while (p < end) {
        if (*p == 0)
            return p + 1;
        if ((*p & 0xC0) == 0xC0)
            return p + 2 <= end ? p + 2 : NULL;
        p += *p + 1;
    }
    return NULL;
}

static int dns_build(uint8_t *out, uint16_t id, const char *name) {
    memset(out, 0, 12);
    out[0] = (uint8_t)(id >> 8);
    out[1] = (uint8_t)id;
    out[2] = DNS_FLAG_RD >> 8;
    out[5] = 1;     // one question

    int at = 12;
    while (*name) {
        const char *dot = strchr(name, '.');
        size_t      len = dot ? (size_t)(dot - name) : strlen(name);
        if (len == 0 || len > 63 || at + (int)len + 6 > DNS_MAX)
            return -1;
        out[at++] = (uint8_t)len;
        memcpy(out + at, name, len);
        at   += (int)len;
        name += len + (dot ? 1 : 0);
    }
    out[at++] = 0;
    out[at++] = 0; out[at++] = DNS_TYPE_A;
    out[at++] = 0; out[at++] = DNS_CLASS_IN;
    return at;
}

// the first A record in the answer, 0 found, EAI_ otherwise
static int dns_parse(const uint8_t *buf, int len, uint16_t id, in_addr_t *out) {
    if (len < 12 || get16(buf) != id)
        return EAI_AGAIN;
    if (get16(buf + 2) & DNS_RCODE_MASK)
        return EAI_NONAME;

    const uint8_t *p   = buf + 12;
    const uint8_t *end = buf + len;
    for (int q = get16(buf + 4); q > 0; q--) {
        p = dns_skip_name(p, end);
        if (!p || p + 4 > end)
            return EAI_FAIL;
        p += 4;
    }

    for (int a = get16(buf + 6); a > 0; a--) {
        p = dns_skip_name(p, end);
        if (!p || p + 10 > end)
            return EAI_FAIL;
        uint16_t type  = get16(p);
        uint16_t class = get16(p + 2);
        uint16_t rdlen = get16(p + 8);
        p += 10;
        if (p + rdlen > end)
            return EAI_FAIL;
        if (type == DNS_TYPE_A && class == DNS_CLASS_IN && rdlen == 4) {
            memcpy(out, p, 4);
            return 0;
        }
        p += rdlen;
    }
    return EAI_NONAME;
}

static int dns_lookup(const char *name, in_addr_t *out) {
    struct net_device_info info;
    if (net_info(&info) != 0 || !info.dns)
        return EAI_AGAIN;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return EAI_SYSTEM;

    struct sockaddr_in server = { .sin_family = AF_INET, .sin_port = htons(DNS_PORT) };
    server.sin_addr.s_addr = info.dns;

    uint8_t query[DNS_MAX], reply[DNS_MAX];
    int     err = EAI_AGAIN;

    for (int tries = 0; tries < DNS_TRIES && err == EAI_AGAIN; tries++) {
        struct timeval now;
        gettimeofday(&now, NULL);
        uint16_t id  = (uint16_t)(now.tv_usec ^ (now.tv_sec << 4) ^ getpid() ^ tries);
        int      len = dns_build(query, id, name);
        if (len < 0) {
            err = EAI_NONAME;
            break;
        }
        if (sendto(fd, query, (size_t)len, 0, (struct sockaddr *)&server, sizeof(server)) < 0)
            break;

        struct pollfd pfd = { .fd = fd, .events = POLLIN };
        while (err == EAI_AGAIN && poll(&pfd, 1, DNS_WAIT_MS) > 0) {
            struct sockaddr_in from;
            socklen_t          from_len = sizeof(from);
            ssize_t            got = recvfrom(fd, reply, sizeof(reply), 0, (struct sockaddr *)&from, &from_len);
            if (got > 0 && from.sin_addr.s_addr == info.dns)
                err = dns_parse(reply, (int)got, id, out);
        }
    }

    close(fd);
    return err;
}

static int resolve(const char *node, int flags, in_addr_t *out) {
    if (!node)
        return EAI_NONAME;

    struct in_addr a;
    if (inet_aton(node, &a)) {
        *out = a.s_addr;
        return 0;
    }
    if (flags & AI_NUMERICHOST)
        return EAI_NONAME;

    if (strcasecmp(node, "localhost") == 0) {
        *out = htonl(INADDR_LOOPBACK);
        return 0;
    }

    // our own name is whatever address dhcp gave us
    char self[256];
    if (gethostname(self, sizeof(self)) == 0 && strcasecmp(node, self) == 0) {
        struct net_device_info info;
        *out = net_info(&info) == 0 && info.ip ? info.ip : htonl(INADDR_LOOPBACK);
        return 0;
    }

    return dns_lookup(node, out);
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    int flags    = hints ? hints->ai_flags : 0;
    int family   = hints ? hints->ai_family : AF_UNSPEC;
    int socktype = hints && hints->ai_socktype ? hints->ai_socktype : SOCK_DGRAM;

    if (!res || (!node && !service))
        return EAI_NONAME;
    *res = NULL;
    if (family != AF_UNSPEC && family != AF_INET)
        return EAI_FAMILY;

    in_port_t port = 0;
    if (service) {
        char *end;
        long  n = strtol(service, &end, 10);
        if (*end || n < 0 || n > 65535)
            return EAI_SERVICE;
        port = htons((uint16_t)n);
    }

    in_addr_t ip;
    if (node) {
        int err = resolve(node, flags, &ip);
        if (err)
            return err;
    } else {
        ip = (flags & AI_PASSIVE) ? htonl(INADDR_ANY) : htonl(INADDR_LOOPBACK);
    }

    struct addrinfo *ai = calloc(1, sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
    if (!ai)
        return EAI_MEMORY;

    struct sockaddr_in *sa = (struct sockaddr_in *)(ai + 1);
    sa->sin_family      = AF_INET;
    sa->sin_port        = port;
    sa->sin_addr.s_addr = ip;

    ai->ai_family   = AF_INET;
    ai->ai_socktype = socktype;
    ai->ai_protocol = socktype == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP;
    ai->ai_addrlen  = sizeof(*sa);
    ai->ai_addr     = (struct sockaddr *)sa;
    *res = ai;
    return 0;
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
    case EAI_SERVICE:  return "Servname not supported";
    case EAI_MEMORY:   return "Memory allocation failure";
    case EAI_SYSTEM:   return "System error";
    default:           return "Unknown error";
    }
}

struct hostent *gethostbyname(const char *name) {
    static char            *addrs[2];
    static in_addr_t        addr;
    static struct hostent   host;
    static char             host_name[256];

    int err = resolve(name, 0, &addr);
    if (err) {
        h_errno = err == EAI_AGAIN ? TRY_AGAIN : HOST_NOT_FOUND;
        return NULL;
    }

    strncpy(host_name, name, sizeof(host_name) - 1);
    addrs[0]         = (char *)&addr;
    addrs[1]         = NULL;
    host.h_name      = host_name;
    host.h_aliases   = &addrs[1];
    host.h_addrtype  = AF_INET;
    host.h_length    = 4;
    host.h_addr_list = addrs;
    return &host;
}
