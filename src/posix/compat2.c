#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <libgen.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <resolv.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>

int getpagesize(void) {
    return 4096;
}

static pthread_mutex_t itimer_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  itimer_cond = PTHREAD_COND_INITIALIZER;
static struct timespec itimer_due;      // 0 when disarmed
static long            itimer_period_ns;
static int             itimer_thread_up;

static long ts_diff_ns(const struct timespec *a, const struct timespec *b) {
    return (a->tv_sec - b->tv_sec) * 1000000000L + (a->tv_nsec - b->tv_nsec);
}

static void ts_add_ns(struct timespec *t, long ns) {
    t->tv_sec += ns / 1000000000L;
    t->tv_nsec += ns % 1000000000L;
    if (t->tv_nsec >= 1000000000L) {
        t->tv_sec++;
        t->tv_nsec -= 1000000000L;
    }
}

static void *itimer_thread(void *arg) {
    (void)arg;
    sigset_t all;
    sigfillset(&all);
    pthread_sigmask(SIG_BLOCK, &all, NULL);

    pthread_mutex_lock(&itimer_lock);
    for (;;) {
        if (!itimer_due.tv_sec && !itimer_due.tv_nsec) {
            pthread_cond_wait(&itimer_cond, &itimer_lock);
            continue;
        }
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        if (ts_diff_ns(&itimer_due, &now) > 0) {
            pthread_cond_timedwait(&itimer_cond, &itimer_lock, &itimer_due);
            continue;
        }
        if (itimer_period_ns) {
            ts_add_ns(&itimer_due, itimer_period_ns);
        } else {
            itimer_due.tv_sec = itimer_due.tv_nsec = 0;
        }
        pthread_mutex_unlock(&itimer_lock);
        kill(getpid(), SIGALRM);
        pthread_mutex_lock(&itimer_lock);
    }
    return NULL;
}

int getitimer(int which, struct itimerval *value) {
    if (which != ITIMER_REAL) {
        errno = EINVAL;
        return -1;
    }
    memset(value, 0, sizeof(*value));
    pthread_mutex_lock(&itimer_lock);
    if (itimer_due.tv_sec || itimer_due.tv_nsec) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        long left = ts_diff_ns(&itimer_due, &now);
        if (left < 1000)
            left = 1000;
        value->it_value.tv_sec = left / 1000000000L;
        value->it_value.tv_usec = (left % 1000000000L) / 1000;
    }
    value->it_interval.tv_sec = itimer_period_ns / 1000000000L;
    value->it_interval.tv_usec = (itimer_period_ns % 1000000000L) / 1000;
    pthread_mutex_unlock(&itimer_lock);
    return 0;
}

int setitimer(int which, const struct itimerval *value, struct itimerval *old) {
    if (which != ITIMER_REAL) {
        errno = EINVAL;
        return -1;
    }
    if (old)
        getitimer(which, old);

    pthread_mutex_lock(&itimer_lock);
    long first = value->it_value.tv_sec * 1000000000L + value->it_value.tv_usec * 1000L;
    itimer_period_ns = value->it_interval.tv_sec * 1000000000L + value->it_interval.tv_usec * 1000L;
    if (first > 0) {
        clock_gettime(CLOCK_REALTIME, &itimer_due);
        ts_add_ns(&itimer_due, first);
    } else {
        itimer_due.tv_sec = itimer_due.tv_nsec = 0;
        itimer_period_ns = 0;
    }
    if (!itimer_thread_up && first > 0) {
        pthread_t t;
        if (pthread_create(&t, NULL, itimer_thread, NULL) == 0) {
            pthread_detach(t);
            itimer_thread_up = 1;
        }
    }
    pthread_cond_signal(&itimer_cond);
    pthread_mutex_unlock(&itimer_lock);
    return 0;
}

unsigned alarm(unsigned secs) {
    struct itimerval v = { { 0, 0 }, { (time_t)secs, 0 } }, old;
    if (setitimer(ITIMER_REAL, &v, &old) != 0)
        return 0;
    return (unsigned)(old.it_value.tv_sec + (old.it_value.tv_usec ? 1 : 0));
}

unsigned ualarm(unsigned usecs, unsigned interval) {
    struct itimerval v = { { 0, (suseconds_t)interval }, { 0, (suseconds_t)usecs } }, old;
    if (setitimer(ITIMER_REAL, &v, &old) != 0)
        return 0;
    return (unsigned)(old.it_value.tv_sec * 1000000 + old.it_value.tv_usec);
}

// no time zones, so mktime already works in utc
time_t timegm(struct tm *tm) {
    return mktime(tm);
}

void *mremap(void *old, size_t old_len, size_t new_len, int flags, ...) {
    (void)flags;
    if (new_len == 0) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    old_len = (old_len + 4095) & ~4095UL;
    new_len = (new_len + 4095) & ~4095UL;
    if (new_len <= old_len) {
        if (new_len < old_len)
            munmap((char *)old + new_len, old_len - new_len);
        return old;
    }
    errno = ENOMEM;
    return MAP_FAILED;
}

int mkfifo(const char *path, mode_t mode) {
    (void)path; (void)mode;
    errno = ENOSYS;
    return -1;
}

int mkfifoat(int dirfd, const char *path, mode_t mode) {
    (void)dirfd;
    return mkfifo(path, mode);
}

int mknod(const char *path, mode_t mode, dev_t dev) {
    (void)path; (void)mode; (void)dev;
    errno = ENOSYS;
    return -1;
}

static void statvfs_to_statfs(const struct statvfs *v, struct statfs *f) {
    memset(f, 0, sizeof(*f));
    f->f_type    = 0x01021994;
    f->f_bsize   = v->f_bsize;
    f->f_frsize  = v->f_frsize;
    f->f_blocks  = v->f_blocks;
    f->f_bfree   = v->f_bfree;
    f->f_bavail  = v->f_bavail;
    f->f_files   = v->f_files;
    f->f_ffree   = v->f_ffree;
    f->f_namelen = v->f_namemax;
    f->f_flags   = v->f_flag;
}

int statfs(const char *path, struct statfs *buf) {
    struct statvfs v;
    if (statvfs(path, &v) != 0)
        return -1;
    statvfs_to_statfs(&v, buf);
    return 0;
}

int fstatfs(int fd, struct statfs *buf) {
    struct statvfs v;
    if (fstatvfs(fd, &v) != 0)
        return -1;
    statvfs_to_statfs(&v, buf);
    return 0;
}

static char *proto_no_aliases[] = { NULL };
static struct protoent protocols[] = {
    { "ip", proto_no_aliases, 0 },
    { "icmp", proto_no_aliases, 1 },
    { "tcp", proto_no_aliases, 6 },
    { "udp", proto_no_aliases, 17 },
    { "ipv6", proto_no_aliases, 41 },
    { "ipv6-icmp", proto_no_aliases, 58 },
};
#define NPROTOCOLS (sizeof(protocols) / sizeof(protocols[0]))
static size_t proto_next;

struct protoent *getprotobyname(const char *name) {
    for (size_t i = 0; i < NPROTOCOLS; i++)
        if (!strcmp(protocols[i].p_name, name))
            return &protocols[i];
    return NULL;
}

struct protoent *getprotobynumber(int proto) {
    for (size_t i = 0; i < NPROTOCOLS; i++)
        if (protocols[i].p_proto == proto)
            return &protocols[i];
    return NULL;
}

struct protoent *getprotoent(void) {
    return proto_next < NPROTOCOLS ? &protocols[proto_next++] : NULL;
}

void setprotoent(int stayopen) {
    (void)stayopen;
    proto_next = 0;
}

void endprotoent(void) {
    proto_next = 0;
}

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type) {
    (void)addr; (void)len; (void)type;
    h_errno = HOST_NOT_FOUND;
    return NULL;
}

int res_nquery(res_state statp, const char *dname, int cls, int type, unsigned char *answer, int anslen) {
    (void)statp;
    return res_query(dname, cls, type, answer, anslen);
}

int res_nsearch(res_state statp, const char *dname, int cls, int type, unsigned char *answer, int anslen) {
    (void)statp;
    return res_query(dname, cls, type, answer, anslen);
}

// one entry, lo with 127.0.0.1, all in one allocation so freeifaddrs is a free
int getifaddrs(struct ifaddrs **list) {
    struct lo_block {
        struct ifaddrs     ifa;
        struct sockaddr_in addr, mask;
        char               name[4];
    } *b = calloc(1, sizeof(*b));
    if (!b) {
        errno = ENOMEM;
        return -1;
    }
    strcpy(b->name, "lo");
    b->addr.sin_family = AF_INET;
    b->addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    b->mask.sin_family = AF_INET;
    b->mask.sin_addr.s_addr = htonl(0xff000000);
    b->ifa.ifa_name = b->name;
    b->ifa.ifa_flags = 0x1 | 0x8 | 0x40;    // IFF_UP | IFF_LOOPBACK | IFF_RUNNING
    b->ifa.ifa_addr = (struct sockaddr *)&b->addr;
    b->ifa.ifa_netmask = (struct sockaddr *)&b->mask;
    *list = &b->ifa;
    return 0;
}

void freeifaddrs(struct ifaddrs *list) {
    free(list);
}

// posix basename/dirname, both can write into path
char *basename(char *path) {
    if (!path || !*path)
        return ".";
    size_t n = strlen(path);
    while (n > 1 && path[n - 1] == '/')
        path[--n] = '\0';
    char *slash = strrchr(path, '/');
    return slash && slash[1] ? slash + 1 : path;
}

char *dirname(char *path) {
    if (!path || !*path)
        return ".";
    size_t n = strlen(path);
    while (n > 1 && path[n - 1] == '/')
        n--;
    while (n > 0 && path[n - 1] != '/')
        n--;
    if (n == 0)
        return ".";
    while (n > 1 && path[n - 1] == '/')
        n--;
    path[n] = '\0';
    return path;
}

int pthread_setaffinity_np(pthread_t thread, size_t size, const cpu_set_t *set) {
    (void)thread;
    return sched_setaffinity(0, size, set) == 0 ? 0 : errno;
}

int pthread_getaffinity_np(pthread_t thread, size_t size, cpu_set_t *set) {
    (void)thread;
    return sched_getaffinity(0, size, set) == 0 ? 0 : errno;
}

char *strsep(char **stringp, const char *delim) {
    char *s = *stringp;
    if (!s)
        return NULL;
    char *end = s + strcspn(s, delim);
    if (*end) {
        *end = '\0';
        *stringp = end + 1;
    } else {
        *stringp = NULL;
    }
    return s;
}

static char *serv_no_aliases[] = { NULL };
static struct servent services[] = {
    { "ftp", serv_no_aliases, 21, "tcp" },
    { "ssh", serv_no_aliases, 22, "tcp" },
    { "smtp", serv_no_aliases, 25, "tcp" },
    { "domain", serv_no_aliases, 53, "udp" },
    { "domain", serv_no_aliases, 53, "tcp" },
    { "http", serv_no_aliases, 80, "tcp" },
    { "https", serv_no_aliases, 443, "tcp" },
    { "https", serv_no_aliases, 443, "udp" },
};
#define NSERVICES (sizeof(services) / sizeof(services[0]))

static struct servent *serv_net_order(const struct servent *s) {
    static struct servent out;
    out = *s;
    out.s_port = htons((uint16_t)s->s_port);
    return &out;
}

struct servent *getservbyname(const char *name, const char *proto) {
    for (size_t i = 0; i < NSERVICES; i++)
        if (!strcmp(services[i].s_name, name) && (!proto || !strcmp(services[i].s_proto, proto)))
            return serv_net_order(&services[i]);
    return NULL;
}

struct servent *getservbyport(int port, const char *proto) {
    int host = ntohs((uint16_t)port);
    for (size_t i = 0; i < NSERVICES; i++)
        if (services[i].s_port == host && (!proto || !strcmp(services[i].s_proto, proto)))
            return serv_net_order(&services[i]);
    return NULL;
}

static volatile int waited_sig;
static void record_sig(int sig) { waited_sig = sig; }

int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout) {
    struct sigaction rec, old[NSIG];
    memset(&rec, 0, sizeof(rec));
    rec.sa_handler = record_sig;
    sigfillset(&rec.sa_mask);
    for (int s = 1; s < NSIG; s++)
        if (sigismember(set, s))
            sigaction(s, &rec, &old[s]);

    sigset_t mask, open_set;
    pthread_sigmask(SIG_BLOCK, NULL, &mask);
    open_set = mask;
    for (int s = 1; s < NSIG; s++)
        if (sigismember(set, s))
            sigdelset(&open_set, s);

    waited_sig = 0;
    int ms = timeout ? (int)(timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000) : -1;
    pthread_sigmask(SIG_SETMASK, &open_set, NULL);
    while (!waited_sig) {
        if (poll(NULL, 0, ms) == 0)
            break;
        if (ms >= 0)
            break;
    }
    pthread_sigmask(SIG_SETMASK, &mask, NULL);

    for (int s = 1; s < NSIG; s++)
        if (sigismember(set, s))
            sigaction(s, &old[s], NULL);

    if (!waited_sig) {
        errno = timeout ? EAGAIN : EINTR;
        return -1;
    }
    if (info) {
        memset(info, 0, sizeof(*info));
        info->si_signo = waited_sig;
    }
    return waited_sig;
}

int sigwaitinfo(const sigset_t *set, siginfo_t *info) {
    return sigtimedwait(set, info, NULL);
}

int sigwait(const sigset_t *set, int *sig) {
    int r;
    do {
        r = sigtimedwait(set, NULL, NULL);
    } while (r < 0 && errno == EINTR);
    if (r < 0)
        return errno;
    *sig = r;
    return 0;
}

int fallocate(int fd, int mode, off_t offset, off_t len) {
    if (mode != 0) {
        errno = EOPNOTSUPP;
        return -1;
    }
    int r = posix_fallocate(fd, offset, len);
    if (r) {
        errno = r;
        return -1;
    }
    return 0;
}

int recvmmsg(int fd, struct mmsghdr *msgs, unsigned int n, int flags, struct timespec *timeout) {
    (void)timeout;
    unsigned int got = 0;
    while (got < n) {
        ssize_t r = recvmsg(fd, &msgs[got].msg_hdr, got ? (flags | MSG_DONTWAIT) : flags);
        if (r < 0)
            return got ? (int)got : -1;
        msgs[got].msg_len = (unsigned int)r;
        got++;
    }
    return (int)got;
}

int sendmmsg(int fd, struct mmsghdr *msgs, unsigned int n, int flags) {
    unsigned int sent = 0;
    while (sent < n) {
        ssize_t r = sendmsg(fd, &msgs[sent].msg_hdr, flags);
        if (r < 0)
            return sent ? (int)sent : -1;
        msgs[sent].msg_len = (unsigned int)r;
        sent++;
    }
    return (int)sent;
}
