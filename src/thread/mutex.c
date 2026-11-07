#include <pthread.h>
#include <errno.h>
#include "thread.h"

int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr) {
    m->lock  = 0;
    m->type  = attr ? attr->type : PTHREAD_MUTEX_NORMAL;
    m->owner = 0;
    m->count = 0;
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *m) {
    return m->lock ? EBUSY : 0;
}

// recursive and errorcheck ones need to know who holds it, normal ones dont bother
static int owner_check(pthread_mutex_t *m, int tid) {
    if (m->type == PTHREAD_MUTEX_NORMAL || m->owner != tid)
        return -1;
    if (m->type == PTHREAD_MUTEX_ERRORCHECK)
        return EDEADLK;
    if (m->count == 0x7fffffff)
        return EAGAIN;
    m->count++;
    return 0;
}

static void took(pthread_mutex_t *m, int tid) {
    if (m->type != PTHREAD_MUTEX_NORMAL) {
        m->owner = tid;
        m->count = 1;
    }
}

int pthread_mutex_lock(pthread_mutex_t *m) {
    int tid = thread_self()->tid;
    int r = owner_check(m, tid);
    if (r >= 0)
        return r;

    lock_take(&m->lock);
    took(m, tid);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *m) {
    int tid = thread_self()->tid;
    int r = owner_check(m, tid);
    if (r >= 0)
        return r == EDEADLK ? EBUSY : r;

    if (!__sync_bool_compare_and_swap(&m->lock, 0, 1))
        return EBUSY;
    took(m, tid);
    return 0;
}

int pthread_mutex_timedlock(pthread_mutex_t *m, const struct timespec *abstime) {
    int tid = thread_self()->tid;
    int r = owner_check(m, tid);
    if (r >= 0)
        return r;

    if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
        return EINVAL;

    while (!__sync_bool_compare_and_swap(&m->lock, 0, 1)) {
        if (__atomic_exchange_n(&m->lock, 2, __ATOMIC_ACQUIRE) == 0)
            break;
        struct timespec rel;
        if (__thread_rel_timeout(CLOCK_REALTIME, abstime, &rel) < 0)
            return ETIMEDOUT;
        futex_wait(&m->lock, 2, &rel);
    }
    took(m, tid);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *m) {
    if (m->type != PTHREAD_MUTEX_NORMAL) {
        if (m->owner != thread_self()->tid)
            return EPERM;
        if (--m->count > 0)
            return 0;
        m->owner = 0;
    }
    lock_give(&m->lock);
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr) {
    attr->type     = PTHREAD_MUTEX_NORMAL;
    attr->pshared  = PTHREAD_PROCESS_PRIVATE;
    attr->protocol = PTHREAD_PRIO_NONE;
    attr->robust   = PTHREAD_MUTEX_STALLED;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) {
    if (type < PTHREAD_MUTEX_NORMAL || type > PTHREAD_MUTEX_ERRORCHECK)
        return EINVAL;
    attr->type = type;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type) {
    *type = attr->type;
    return 0;
}

// TODO: shared ones need the futex without the private flag
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared) {
    if (pshared != PTHREAD_PROCESS_PRIVATE)
        return ENOTSUP;
    attr->pshared = pshared;
    return 0;
}

int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared) {
    *pshared = attr->pshared;
    return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol) {
    if (protocol != PTHREAD_PRIO_NONE)
        return ENOTSUP;
    attr->protocol = protocol;
    return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol) {
    *protocol = attr->protocol;
    return 0;
}

int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust) {
    if (robust != PTHREAD_MUTEX_STALLED)
        return ENOTSUP;
    attr->robust = robust;
    return 0;
}

int pthread_mutexattr_getrobust(const pthread_mutexattr_t *attr, int *robust) {
    *robust = attr->robust;
    return 0;
}
