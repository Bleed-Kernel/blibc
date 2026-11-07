#include <errno.h>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/auxv.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/file.h>
#include <sys/sendfile.h>
#include <sys/shm.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <resolv.h>
#include <netinet/ether.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

extern unsigned long *__blibc_auxv;

unsigned long getauxval(unsigned long type) {
    for (unsigned long *a = __blibc_auxv; a && a[0] != AT_NULL; a += 2) {
        if (a[0] == type)
            return a[1];
    }
    errno = ENOENT;
    return 0;
}

// the kernel takes linux numbers for everything under 512
long syscall(long n, ...) {
    va_list ap;
    va_start(ap, n);
    long a = va_arg(ap, long), b = va_arg(ap, long), c = va_arg(ap, long);
    long d = va_arg(ap, long), e = va_arg(ap, long), f = va_arg(ap, long);
    va_end(ap);

    long ret;
    register long r10 __asm__("r10") = d;
    register long r8  __asm__("r8")  = e;
    register long r9  __asm__("r9")  = f;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9)
                     : "rcx", "r11", "memory");
    if (ret < 0 && ret > -4096) {
        errno = (int)-ret;
        return -1;
    }
    return ret;
}

int sched_getaffinity(pid_t pid, size_t size, cpu_set_t *set) {
    long ret = _sched_getaffinity(pid, size, set);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    if ((size_t)ret < size)
        memset((char *)set + ret, 0, size - (size_t)ret);
    return 0;
}

// everything runs everywhere
int sched_setaffinity(pid_t pid, size_t size, const cpu_set_t *set) {
    (void)pid; (void)size; (void)set;
    return 0;
}

int sched_getcpu(void) {
    return 0;
}

int __sched_cpucount(size_t size, const cpu_set_t *set) {
    int n = 0;
    // by hand, the builtin turns into a libgcc call and not everything links libgcc
    for (size_t i = 0; i < size / sizeof(long); i++)
        for (unsigned long b = set->__bits[i]; b; b &= b - 1)
            n++;
    return n;
}

int pause(void) {
    poll(NULL, 0, -1);
    errno = EINTR;
    return -1;
}

// theres one user and its root, so root can become root
static int only_root(unsigned id) {
    if (id == 0 || id == (unsigned)-1)
        return 0;
    errno = EPERM;
    return -1;
}

int setuid(uid_t uid)  { return only_root(uid); }
int setgid(gid_t gid)  { return only_root(gid); }
int seteuid(uid_t uid) { return only_root(uid); }
int setegid(gid_t gid) { return only_root(gid); }
int setreuid(uid_t r, uid_t e) { return only_root(r) ? -1 : only_root(e); }
int setregid(gid_t r, gid_t e) { return only_root(r) ? -1 : only_root(e); }

int getgroups(int size, gid_t list[]) {
    if (size > 0)
        list[0] = 0;
    return 1;
}

int setgroups(size_t size, const gid_t *list) {
    for (size_t i = 0; i < size; i++) {
        if (only_root(list[i]))
            return -1;
    }
    return 0;
}

int chroot(const char *path) {
    (void)path;
    errno = EPERM;
    return -1;
}

// P_PID and P_ALL, WNOWAIT leaves the child to be waited for again
int waitid(idtype_t type, id_t id, siginfo_t *info, int options) {
    long pid;
    if (type == P_PID)
        pid = (long)id;
    else if (type == P_ALL)
        pid = -1;
    else {
        errno = EINVAL;
        return -1;
    }
    if (!(options & WEXITED) || (options & ~(WEXITED | WNOHANG | WNOWAIT | WSTOPPED | WCONTINUED))) {
        errno = EINVAL;
        return -1;
    }

    int  status = 0;
    long got = _waitopts(pid, &status, options & (WNOHANG | WNOWAIT));
    if (got < 0) {
        errno = (int)-got;
        return -1;
    }

    memset(info, 0, sizeof(*info));
    if (got == 0)
        return 0;
    info->si_signo = SIGCHLD;
    info->si_pid   = (pid_t)got;
    if (WIFEXITED(status)) {
        info->si_code   = CLD_EXITED;
        info->si_status = WEXITSTATUS(status);
    } else {
        info->si_code   = CLD_KILLED;
        info->si_status = WTERMSIG(status);
    }
    return 0;
}

int getrlimit(int resource, struct rlimit *rlim) {
    switch (resource) {
    case RLIMIT_STACK:
        rlim->rlim_cur = 8 * 1024 * 1024;
        rlim->rlim_max = RLIM_INFINITY;
        return 0;
    case RLIMIT_NOFILE:
        rlim->rlim_cur = rlim->rlim_max = 256;
        return 0;
    case RLIMIT_CORE:
        rlim->rlim_cur = rlim->rlim_max = 0;
        return 0;
    default:
        if (resource < 0 || resource >= RLIMIT_NLIMITS) {
            errno = EINVAL;
            return -1;
        }
        rlim->rlim_cur = rlim->rlim_max = RLIM_INFINITY;
        return 0;
    }
}

// asking for less than there is is fine, theres nothing to enforce it with
int setrlimit(int resource, const struct rlimit *rlim) {
    struct rlimit cur;
    if (getrlimit(resource, &cur) < 0)
        return -1;
    if (rlim->rlim_max != RLIM_INFINITY && rlim->rlim_cur > rlim->rlim_max) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int prlimit(pid_t pid, int resource, const struct rlimit *new_limit, struct rlimit *old_limit) {
    (void)pid;
    if (old_limit && getrlimit(resource, old_limit) < 0)
        return -1;
    if (new_limit)
        return setrlimit(resource, new_limit);
    return 0;
}

int getrusage(int who, struct rusage *usage) {
    (void)who;
    memset(usage, 0, sizeof(*usage));
    return 0;
}

int getpriority(int which, id_t who) {
    (void)which; (void)who;
    errno = 0;
    return 0;
}

int setpriority(int which, id_t who, int prio) {
    (void)which; (void)who; (void)prio;
    return 0;
}

int __xpg_strerror_r(int errnum, char *buf, size_t len) {
    return strerror_r(errnum, buf, len);
}

static __thread char prctl_name[16];

int prctl(int option, ...) {
    va_list ap;
    va_start(ap, option);
    unsigned long a = va_arg(ap, unsigned long);
    va_end(ap);
    switch (option) {
    case PR_SET_NAME:
        strncpy(prctl_name, (const char *)a, sizeof(prctl_name) - 1);
        return 0;
    case PR_GET_NAME:
        memcpy((char *)a, prctl_name, sizeof(prctl_name));
        return 0;
    case PR_GET_DUMPABLE:
        return 1;
    case PR_SET_DUMPABLE: case PR_SET_PDEATHSIG: case PR_SET_PTRACER: case PR_SET_NO_NEW_PRIVS:
    case PR_SET_TIMERSLACK: case PR_SET_CHILD_SUBREAPER: case PR_SET_VMA: case PR_SET_THP_DISABLE:
        return 0;
    case PR_GET_NO_NEW_PRIVS: case PR_GET_PDEATHSIG: case PR_GET_CHILD_SUBREAPER: case PR_GET_SECCOMP:
        return 0;
    }
    errno = EINVAL;
    return -1;
}

long ptrace(int request, ...) {
    (void)request;
    errno = EPERM;
    return -1;
}

ssize_t readahead(int fd, off_t offset, size_t count) {
    (void)offset; (void)count;
    struct stat st;
    return fstat(fd, &st) == 0 ? 0 : -1;
}

int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
    (void)fd; (void)offset; (void)len; (void)advice;
    return 0;
}

// grows the file, theres no sparse files to fill in
int posix_fallocate(int fd, off_t offset, off_t len) {
    struct stat st;
    if (offset < 0 || len <= 0)
        return EINVAL;
    if (fstat(fd, &st) != 0)
        return errno;
    if (st.st_size < offset + len && ftruncate(fd, offset + len) != 0)
        return errno;
    return 0;
}

int flock(int fd, int op) {
    (void)op;
    struct stat st;
    return fstat(fd, &st);
}

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
    char buf[4096];
    size_t done = 0;
    off_t at = offset ? *offset : 0;
    while (done < count) {
        size_t want = count - done < sizeof(buf) ? count - done : sizeof(buf);
        ssize_t r = offset ? pread(in_fd, buf, want, at) : read(in_fd, buf, want);
        if (r < 0) {
            if (done)
                break;
            return -1;
        }
        if (r == 0)
            break;
        ssize_t w = 0;
        while (w < r) {
            ssize_t n = write(out_fd, buf + w, (size_t)(r - w));
            if (n < 0) {
                if (done + w)
                    goto out;
                return -1;
            }
            w += n;
        }
        done += (size_t)r;
        at += r;
    }
out:
    if (offset)
        *offset = at;
    return (ssize_t)done;
}

ssize_t preadv(int fd, const struct iovec *iov, int count, off_t offset) {
    ssize_t total = 0;
    for (int i = 0; i < count; i++) {
        ssize_t r = pread(fd, iov[i].iov_base, iov[i].iov_len, offset + total);
        if (r < 0)
            return total ? total : -1;
        total += r;
        if ((size_t)r < iov[i].iov_len)
            break;
    }
    return total;
}

ssize_t pwritev(int fd, const struct iovec *iov, int count, off_t offset) {
    ssize_t total = 0;
    for (int i = 0; i < count; i++) {
        ssize_t r = pwrite(fd, iov[i].iov_base, iov[i].iov_len, offset + total);
        if (r < 0)
            return total ? total : -1;
        total += r;
        if ((size_t)r < iov[i].iov_len)
            break;
    }
    return total;
}

int shmget(key_t key, size_t size, int flag) {
    (void)key; (void)size; (void)flag;
    errno = ENOSYS;
    return -1;
}

void *shmat(int id, const void *addr, int flag) {
    (void)id; (void)addr; (void)flag;
    errno = ENOSYS;
    return (void *)-1;
}

int shmdt(const void *addr) {
    (void)addr;
    errno = EINVAL;
    return -1;
}

int shmctl(int id, int cmd, struct shmid_ds *buf) {
    (void)id; (void)cmd; (void)buf;
    errno = EINVAL;
    return -1;
}

// no getcontext/setcontext, makecontext style coroutines arent used by anything here
int getcontext(ucontext_t *uc) {
    (void)uc;
    errno = ENOSYS;
    return -1;
}

int setcontext(const ucontext_t *uc) {
    (void)uc;
    errno = ENOSYS;
    return -1;
}

int res_ninit(res_state statp) {
    memset(statp, 0, sizeof(*statp));
    statp->retrans = 5;
    statp->retry = 2;
    statp->options = RES_INIT | RES_DEFAULT;
    statp->ndots = 1;
    return 0;
}

int res_init(void) {
    return 0;
}

void res_nclose(res_state statp) {
    (void)statp;
}

int res_query(const char *dname, int cls, int type, unsigned char *answer, int anslen) {
    (void)dname; (void)cls; (void)type; (void)answer; (void)anslen;
    errno = ENOSYS;
    return -1;
}

int res_search(const char *dname, int cls, int type, unsigned char *answer, int anslen) {
    return res_query(dname, cls, type, answer, anslen);
}

char *ether_ntoa_r(const struct ether_addr *addr, char *buf) {
    const uint8_t *o = addr->ether_addr_octet;
    sprintf(buf, "%x:%x:%x:%x:%x:%x", o[0], o[1], o[2], o[3], o[4], o[5]);
    return buf;
}

char *ether_ntoa(const struct ether_addr *addr) {
    static char buf[18];
    return ether_ntoa_r(addr, buf);
}

struct ether_addr *ether_aton_r(const char *s, struct ether_addr *addr) {
    for (int i = 0; i < ETH_ALEN; i++) {
        char *end;
        unsigned long v = strtoul(s, &end, 16);
        if (end == s || v > 0xff || (i < ETH_ALEN - 1 ? *end != ':' : *end != '\0'))
            return NULL;
        addr->ether_addr_octet[i] = (uint8_t)v;
        s = end + 1;
    }
    return addr;
}

struct ether_addr *ether_aton(const char *s) {
    static struct ether_addr addr;
    return ether_aton_r(s, &addr);
}
