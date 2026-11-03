#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
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

// /initrd/etc/hosts, then A records from /initrd/etc/resolv.conf or the dns server dhcp gave the kernel, no cache tho

#define DNS_PORT        53
#define DNS_TRIES       2
#define DNS_WAIT_MS     1500
#define DNS_MAX         512     // a plain udp answer never goes past this
#define DNS_TCP_MAX     8192    // too big for udp comes over tcp, way past enough addresses
#define DNS_MAX_ADDRS   8
#define DNS_MAX_SERVERS 3       // like glibc
#define RESOLV_PATH     "/initrd/etc/resolv.conf"
#define DNS_TYPE_A      1
#define DNS_CLASS_IN    1
#define DNS_FLAG_RD     0x0100
#define DNS_FLAG_TC     0x0200
#define DNS_RCODE_MASK  0x000F

#define HOSTS_PATH      "/initrd/etc/hosts"
#define HOSTS_LINE      512

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

// every A record in the answer, the count in *found. 0 ok, EAI_ otherwise
static int dns_parse(const uint8_t *buf, int len, uint16_t id, in_addr_t *out, int *found) {
    *found = 0;
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
        if (type == DNS_TYPE_A && class == DNS_CLASS_IN && rdlen == 4 && *found < DNS_MAX_ADDRS)
            memcpy(&out[(*found)++], p, 4);
        p += rdlen;
    }
    return *found ? 0 : EAI_NONAME;
}

static int tcp_read_all(int fd, uint8_t *buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t r = recv(fd, buf + got, len - got, 0);
        if (r <= 0)
            return -1;
        got += (size_t)r;
    }
    return 0;
}

// same question again over tcp, the answer has a two byte length in front
static int dns_lookup_tcp(in_addr_t server_ip, const uint8_t *query, int qlen, uint16_t id, in_addr_t *out, int *found) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return EAI_SYSTEM;

    struct timeval wait = { .tv_sec = 5 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &wait, sizeof(wait));

    struct sockaddr_in server = { .sin_family = AF_INET, .sin_port = htons(DNS_PORT) };
    server.sin_addr.s_addr = server_ip;

    int      err = EAI_AGAIN;
    uint8_t  head[2 + DNS_MAX];
    uint8_t *reply = malloc(DNS_TCP_MAX);

    head[0] = (uint8_t)(qlen >> 8);
    head[1] = (uint8_t)qlen;
    memcpy(head + 2, query, (size_t)qlen);

    if (reply && connect(fd, (struct sockaddr *)&server, sizeof(server)) == 0 &&
        send(fd, head, (size_t)qlen + 2, 0) == qlen + 2 && tcp_read_all(fd, head, 2) == 0) {
        int len = get16(head);
        if (len <= DNS_TCP_MAX && tcp_read_all(fd, reply, (size_t)len) == 0)
            err = dns_parse(reply, len, id, out, found);
    }

    free(reply);
    close(fd);
    return err;
}

// nameservers out of /etc/resolv.conf, the dhcp one when that has none
static int dns_servers(in_addr_t *servers, int *wait_ms, int *tries) {
    int count = 0;
    *wait_ms  = DNS_WAIT_MS;
    *tries    = DNS_TRIES;

    FILE *f = fopen(RESOLV_PATH, "r");
    if (f) {
        char line[HOSTS_LINE];
        while (fgets(line, sizeof(line), f)) {
            char *at   = line;
            char *word = strtok_r(line, " \t\r\n", &at);
            if (!word || word[0] == '#' || word[0] == ';')
                continue;

            if (strcmp(word, "nameserver") == 0) {
                struct in_addr a;
                char *ip = strtok_r(NULL, " \t\r\n", &at);
                if (ip && inet_aton(ip, &a) && count < DNS_MAX_SERVERS)
                    servers[count++] = a.s_addr;
            } else if (strcmp(word, "options") == 0) {
                while ((word = strtok_r(NULL, " \t\r\n", &at))) {
                    if (strncmp(word, "timeout:", 8) == 0 && atoi(word + 8) > 0)
                        *wait_ms = atoi(word + 8) * 1000;
                    else if (strncmp(word, "attempts:", 9) == 0 && atoi(word + 9) > 0)
                        *tries = atoi(word + 9);
                }
            }
        }
        fclose(f);
    }

    struct net_device_info info;
    if (!count && net_info(&info) == 0 && info.dns)
        servers[count++] = info.dns;
    return count;
}

// one question to one server, EAI_AGAIN when it didnt answer in time
static int dns_ask(int fd, in_addr_t server_ip, const char *name, int wait_ms, in_addr_t *out, int *found) {
    struct sockaddr_in server = { .sin_family = AF_INET, .sin_port = htons(DNS_PORT) };
    server.sin_addr.s_addr = server_ip;

    uint8_t  query[DNS_MAX], reply[DNS_MAX];
    uint16_t id  = (uint16_t)arc4random();
    int      len = dns_build(query, id, name);
    if (len < 0)
        return EAI_NONAME;
    if (sendto(fd, query, (size_t)len, 0, (struct sockaddr *)&server, sizeof(server)) < 0)
        return EAI_AGAIN;

    int err = EAI_AGAIN;
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    while (err == EAI_AGAIN && poll(&pfd, 1, wait_ms) > 0) {
        struct sockaddr_in from;
        socklen_t          from_len = sizeof(from);
        ssize_t            got = recvfrom(fd, reply, sizeof(reply), 0, (struct sockaddr *)&from, &from_len);
        if (got <= 0 || from.sin_addr.s_addr != server_ip || from.sin_port != server.sin_port)
            continue;

        if (got >= 12 && get16(reply) == id && (get16(reply + 2) & DNS_FLAG_TC)) {
            err = dns_lookup_tcp(server_ip, query, len, id, out, found);
            if (err == EAI_AGAIN)
                err = dns_parse(reply, (int)got, id, out, found);
        } else {
            err = dns_parse(reply, (int)got, id, out, found);
        }
    }
    return err;
}

static int dns_lookup(const char *name, in_addr_t *out, int *found) {
    in_addr_t servers[DNS_MAX_SERVERS];
    int       wait_ms, tries;
    int       count = dns_servers(servers, &wait_ms, &tries);
    if (!count)
        return EAI_AGAIN;

    // a fresh socket gets a fresh random port, one more thing a spoofer has to guess
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return EAI_SYSTEM;

    int err = EAI_AGAIN;
    for (int t = 0; t < tries && err == EAI_AGAIN; t++) {
        for (int s = 0; s < count && err == EAI_AGAIN; s++)
            err = dns_ask(fd, servers[s], name, wait_ms, out, found);
    }

    close(fd);
    return err;
}

// addr name name..., # to the end of the line is a comment
static int hosts_lookup(const char *name, in_addr_t *out, int *found) {
    FILE *f = fopen(HOSTS_PATH, "r");
    if (!f)
        return 0;

    char line[HOSTS_LINE];
    while (fgets(line, sizeof(line), f)) {
        char *hash = strchr(line, '#');
        if (hash)
            *hash = 0;

        char *at = line;
        char *word = strtok_r(line, " \t\r\n", &at);
        struct in_addr a;
        if (!word || !inet_aton(word, &a))
            continue;

        while ((word = strtok_r(NULL, " \t\r\n", &at))) {
            if (strcasecmp(word, name) == 0 && *found < DNS_MAX_ADDRS) {
                out[(*found)++] = a.s_addr;
                break;
            }
        }
    }

    fclose(f);
    return *found;
}

static int resolve(const char *node, int flags, in_addr_t *out, int *found) {
    if (!node)
        return EAI_NONAME;

    *found = 1;
    struct in_addr a;
    if (inet_aton(node, &a)) {
        *out = a.s_addr;
        return 0;
    }
    if (flags & AI_NUMERICHOST)
        return EAI_NONAME;

    *found = 0;
    if (hosts_lookup(node, out, found))
        return 0;

    *found = 1;
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

    return dns_lookup(node, out, found);
}

static const struct { const char *name; uint16_t port; } services[] = {
    { "ftp", 21 }, { "ssh", 22 }, { "telnet", 23 }, { "smtp", 25 }, { "domain", 53 },
    { "gopher", 70 }, { "http", 80 }, { "pop3", 110 }, { "imap", 143 }, { "https", 443 },
    { "smtps", 465 }, { "imaps", 993 }, { "pop3s", 995 },
};

static int service_port(const char *service, int flags, in_port_t *port) {
    char *end;
    long  n = strtol(service, &end, 10);
    if (*service && !*end) {
        if (n < 0 || n > 65535)
            return EAI_SERVICE;
        *port = htons((uint16_t)n);
        return 0;
    }
    if (flags & AI_NUMERICSERV)
        return EAI_NONAME;

    for (size_t i = 0; i < sizeof(services) / sizeof(services[0]); i++) {
        if (strcasecmp(service, services[i].name) == 0) {
            *port = htons(services[i].port);
            return 0;
        }
    }
    return EAI_SERVICE;
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
        int err = service_port(service, flags, &port);
        if (err)
            return err;
    }

    in_addr_t ips[DNS_MAX_ADDRS];
    int       count = 1;
    if (node) {
        int err = resolve(node, flags, ips, &count);
        if (err)
            return err;
    } else {
        ips[0] = (flags & AI_PASSIVE) ? htonl(INADDR_ANY) : htonl(INADDR_LOOPBACK);
    }

    struct addrinfo **tail = res;
    for (int i = 0; i < count; i++) {
        struct addrinfo *ai = calloc(1, sizeof(struct addrinfo) + sizeof(struct sockaddr_in));
        if (!ai) {
            freeaddrinfo(*res);
            *res = NULL;
            return EAI_MEMORY;
        }

        struct sockaddr_in *sa = (struct sockaddr_in *)(ai + 1);
        sa->sin_family      = AF_INET;
        sa->sin_port        = port;
        sa->sin_addr.s_addr = ips[i];

        ai->ai_family   = AF_INET;
        ai->ai_socktype = socktype;
        ai->ai_protocol = socktype == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP;
        ai->ai_addrlen  = sizeof(*sa);
        ai->ai_addr     = (struct sockaddr *)sa;
        *tail = ai;
        tail  = &ai->ai_next;
    }
    return 0;
}

// numbers only, theres nothing doing reverse lookups
int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags) {
    if (!addr || addrlen < sizeof(struct sockaddr_in) || addr->sa_family != AF_INET)
        return EAI_FAMILY;
    if (flags & NI_NAMEREQD)
        return EAI_NONAME;

    const struct sockaddr_in *in = (const struct sockaddr_in *)addr;
    if (host && hostlen && !inet_ntop(AF_INET, &in->sin_addr, host, hostlen))
        return EAI_OVERFLOW;
    if (serv && servlen && snprintf(serv, servlen, "%u", ntohs(in->sin_port)) >= (int)servlen)
        return EAI_OVERFLOW;
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
    case EAI_NODATA:   return "No address associated with hostname";
    case EAI_FAMILY:   return "Address family not supported";
    case EAI_SERVICE:  return "Servname not supported";
    case EAI_MEMORY:   return "Memory allocation failure";
    case EAI_SYSTEM:   return "System error";
    case EAI_OVERFLOW: return "Buffer too small";
    default:           return "Unknown error";
    }
}

struct hostent *gethostbyname(const char *name) {
    static char            *addrs[DNS_MAX_ADDRS + 1];
    static in_addr_t        ips[DNS_MAX_ADDRS];
    static struct hostent   host;
    static char             host_name[256];

    int count;
    int err = resolve(name, 0, ips, &count);
    if (err) {
        h_errno = err == EAI_AGAIN ? TRY_AGAIN : HOST_NOT_FOUND;
        return NULL;
    }

    strncpy(host_name, name, sizeof(host_name) - 1);
    for (int i = 0; i < count; i++)
        addrs[i] = (char *)&ips[i];
    addrs[count]     = NULL;
    host.h_name      = host_name;
    host.h_aliases   = &addrs[count];
    host.h_addrtype  = AF_INET;
    host.h_length    = 4;
    host.h_addr_list = addrs;
    return &host;
}
