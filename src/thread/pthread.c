#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <bleed/syscalls.h>
#include "thread.h"

#define PAGE_SIZE           4096
#define DEFAULT_STACK       (512 * 1024)

#define CLONE_VM            0x00000100
#define CLONE_FS            0x00000200
#define CLONE_FILES         0x00000400
#define CLONE_SIGHAND       0x00000800
#define CLONE_THREAD        0x00010000
#define CLONE_SYSVSEM       0x00040000
#define CLONE_SETTLS        0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000

#define CLONE_PTHREAD       (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | \
                             CLONE_SYSVSEM | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID)

volatile int __thread_count = 1;

static int thread_main(void *p) {
    struct __pthread *self = p;
    pthread_exit(self->start(self->arg));
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg) {
    pthread_attr_t def;
    if (!attr) {
        pthread_attr_init(&def);
        attr = &def;
    }

    size_t stack = attr->stack_size ? attr->stack_size : DEFAULT_STACK;
    size_t guard = (attr->guard_size + PAGE_SIZE - 1) & ~(size_t)(PAGE_SIZE - 1);
    stack = (stack + PAGE_SIZE - 1) & ~(size_t)(PAGE_SIZE - 1);
    size_t size = guard + stack + ((__blibc_tls_size() + PAGE_SIZE - 1) & ~(size_t)(PAGE_SIZE - 1));

    // TODO: a caller given stack (attr->stack_addr) still gets its own mapping
    void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (map == MAP_FAILED)
        return EAGAIN;

    // an overflow hits this instead of the mapping below
    if (guard && mprotect(map, guard, PROT_NONE) < 0) {
        munmap(map, size);
        return EAGAIN;
    }

    struct __pthread *t = __blibc_tls_setup(map, size);
    t->start      = start;
    t->arg        = arg;
    t->map        = map;
    t->map_size   = size;
    t->stack_low  = (char *)map + guard;
    t->state      = attr->detached ? THREAD_DETACHED : THREAD_JOINABLE;

    // the stack tops out just under the tls block
    uintptr_t sp = ((uintptr_t)map + size - __blibc_tls_size()) & ~(uintptr_t)15;
    __blibc_tls_thread_add(t);

    t->stack_size = sp - (uintptr_t)t->stack_low;

    __atomic_add_fetch(&__thread_count, 1, __ATOMIC_SEQ_CST);
    long ret = __thread_clone(CLONE_PTHREAD, (void *)sp, &t->tid, &t->tid, t, thread_main, t);
    if (ret < 0) {
        __blibc_tls_thread_remove(t);
        __atomic_sub_fetch(&__thread_count, 1, __ATOMIC_SEQ_CST);
        munmap(map, size);
        return ret == -ENOMEM ? EAGAIN : (int)-ret;
    }

    *thread = t;
    return 0;
}

void pthread_exit(void *result) {
    struct __pthread *self = thread_self();
    self->result = result;

    __blibc_run_tls_dtors();
    while (self->cleanup) {
        struct __pthread_cleanup *c = self->cleanup;
        self->cleanup = c->next;
        c->fn(c->arg);
    }
    if (self->tsd_used)
        __pthread_tsd_run(self);
    __blibc_tls_thread_remove(self);

    // last one out takes the process with it, same as glibc
    if (__atomic_sub_fetch(&__thread_count, 1, __ATOMIC_SEQ_CST) == 0)
        exit(0);

    int old = THREAD_JOINABLE;
    if (!__atomic_compare_exchange_n(&self->state, &old, THREAD_EXITED, 0,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE) && self->map) {

        sigset_t all;
        sigfillset(&all);
        sigprocmask(SIG_SETMASK, &all, NULL);
        thread_syscall6(SYS_SET_TID_ADDRESS, 0, 0, 0, 0, 0, 0);
        __thread_unmap_and_exit(self->map, self->map_size);
    }

    for (;;)
        thread_syscall6(60, 0, 0, 0, 0, 0, 0);
}

int pthread_join(pthread_t t, void **result) {
    if (t == thread_self())
        return EDEADLK;
    if (t->state == THREAD_DETACHED)
        return EINVAL;

    int tid;
    while ((tid = t->tid) != 0)
        futex_wait(&t->tid, tid, NULL);

    if (result)
        *result = t->result;
    if (t->map)
        munmap(t->map, t->map_size);
    return 0;
}

int pthread_detach(pthread_t t) {
    int old = THREAD_JOINABLE;
    if (__atomic_compare_exchange_n(&t->state, &old, THREAD_DETACHED, 0,
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return 0;
    if (old == THREAD_DETACHED)
        return EINVAL;

    return pthread_join(t, NULL);
}

pthread_t pthread_self(void) {
    return thread_self();
}

int pthread_equal(pthread_t a, pthread_t b) {
    return a == b;
}

pid_t pthread_gettid_np(pthread_t t) {
    return t->tid;
}

int pthread_kill(pthread_t t, int sig) {
    int tid = t->tid;
    if (!tid)
        return ESRCH;
    long ret = thread_syscall6(SYS_TGKILL, getpid(), tid, sig, 0, 0, 0);
    return ret < 0 ? (int)-ret : 0;
}

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
    // sigprocmask is per thread in the kernel already
    int saved = errno;
    if (sigprocmask(how, set, oldset) < 0) {
        int err = errno;
        errno = saved;
        return err;
    }
    return 0;
}

void __pthread_cleanup_push(struct __pthread_cleanup *c, void (*fn)(void *), void *arg) {
    struct __pthread *self = thread_self();
    c->fn   = fn;
    c->arg  = arg;
    c->next = self->cleanup;
    self->cleanup = c;
}

void __pthread_cleanup_pop(struct __pthread_cleanup *c, int run) {
    thread_self()->cleanup = c->next;
    if (run)
        c->fn(c->arg);
}

int pthread_once(pthread_once_t *once, void (*fn)(void)) {
    volatile int *o = once;
    // 0 not run, 1 running, 2 done, 3 running with someone waiting
    for (;;) {
        int s = __atomic_load_n(o, __ATOMIC_ACQUIRE);
        if (s == 2)
            return 0;
        if (s == 0) {
            if (!__sync_bool_compare_and_swap(o, 0, 1))
                continue;
            fn();
            if (__atomic_exchange_n(o, 2, __ATOMIC_RELEASE) == 3)
                futex_wake(o, 0x7fffffff);
            return 0;
        }
        if (s == 1)
            __sync_bool_compare_and_swap(o, 1, 3);
        futex_wait(o, 3, NULL);
    }
}

int pthread_setname_np(pthread_t t, const char *name) {
    if (strlen(name) >= sizeof(t->name))
        return ERANGE;
    strcpy(t->name, name);
    return 0;
}

int pthread_getname_np(pthread_t t, char *name, size_t len) {
    if (strlen(t->name) >= len)
        return ERANGE;
    strcpy(name, t->name);
    return 0;
}

int pthread_getattr_np(pthread_t t, pthread_attr_t *attr) {
    pthread_attr_init(attr);
    attr->detached   = t->state == THREAD_DETACHED;
    attr->stack_addr = t->stack_low;
    attr->stack_size = t->stack_size;
    return 0;
}

int pthread_getconcurrency(void) {
    return 0;
}

int pthread_setconcurrency(int level) {
    return level < 0 ? EINVAL : 0;
}

// theres one scheduling class, so these just report it
int pthread_getschedparam(pthread_t t, int *policy, struct sched_param *param) {
    (void)t;
    *policy = SCHED_OTHER;
    param->sched_priority = 0;
    return 0;
}

int pthread_setschedparam(pthread_t t, int policy, const struct sched_param *param) {
    (void)t;
    return policy == SCHED_OTHER && param->sched_priority == 0 ? 0 : ENOTSUP;
}

int pthread_setschedprio(pthread_t t, int prio) {
    (void)t;
    return prio == 0 ? 0 : ENOTSUP;
}

int pthread_getcpuclockid(pthread_t t, clockid_t *clock) {
    (void)t;
    (void)clock;
    return ENOENT;
}

int pthread_cancel(pthread_t t) {
    (void)t;
    return ENOSYS;
}

int pthread_setcancelstate(int state, int *oldstate) {
    if (oldstate)
        *oldstate = PTHREAD_CANCEL_ENABLE;
    return state == PTHREAD_CANCEL_ENABLE || state == PTHREAD_CANCEL_DISABLE ? 0 : EINVAL;
}

int pthread_setcanceltype(int type, int *oldtype) {
    if (oldtype)
        *oldtype = PTHREAD_CANCEL_DEFERRED;
    return type == PTHREAD_CANCEL_DEFERRED || type == PTHREAD_CANCEL_ASYNCHRONOUS ? 0 : EINVAL;
}

void pthread_testcancel(void) {
}

int sched_yield(void) {
    _yeild();
    return 0;
}

int sched_get_priority_max(int policy) {
    (void)policy;
    return 0;
}

int sched_get_priority_min(int policy) {
    (void)policy;
    return 0;
}

int sched_getparam(pid_t pid, struct sched_param *param) {
    (void)pid;
    param->sched_priority = 0;
    return 0;
}

int sched_setparam(pid_t pid, const struct sched_param *param) {
    (void)pid;
    (void)param;
    return 0;
}

int sched_getscheduler(pid_t pid) {
    (void)pid;
    return SCHED_OTHER;
}

int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param) {
    (void)pid;
    (void)param;
    if (policy != SCHED_OTHER) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int __thread_rel_timeout(clockid_t clock, const struct timespec *abstime, struct timespec *rel) {
    struct timespec now;
    clock_gettime(clock, &now);
    rel->tv_sec  = abstime->tv_sec - now.tv_sec;
    rel->tv_nsec = abstime->tv_nsec - now.tv_nsec;
    if (rel->tv_nsec < 0) {
        rel->tv_nsec += 1000000000;
        rel->tv_sec--;
    }
    return rel->tv_sec < 0 ? -1 : 0;
}
