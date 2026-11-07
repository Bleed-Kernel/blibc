#include <pthread.h>
#include <errno.h>
#include "thread.h"

int pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *attr) {
    c->seq   = 0;
    c->clock = attr ? attr->clock : CLOCK_REALTIME;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *c) {
    (void)c;
    return 0;
}

static int cond_wait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *abstime) {
    struct timespec rel;
    if (abstime && (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000))
        return EINVAL;

    int seq = c->seq;

    int count = m->count;
    if (m->type != PTHREAD_MUTEX_NORMAL) {
        if (m->owner != thread_self()->tid)
            return EPERM;
        m->count = 1;
    }
    pthread_mutex_unlock(m);

    int ret = 0;
    if (abstime && __thread_rel_timeout(c->clock, abstime, &rel) < 0)
        ret = ETIMEDOUT;
    else if (futex_wait(&c->seq, seq, abstime ? &rel : NULL) == -ETIMEDOUT)
        ret = ETIMEDOUT;

    // as if contended, so whoever unlocks after us wakes the next sleeper
    while (__atomic_exchange_n(&m->lock, 2, __ATOMIC_ACQUIRE) != 0)
        futex_wait(&m->lock, 2, NULL);
    if (m->type != PTHREAD_MUTEX_NORMAL) {
        m->owner = thread_self()->tid;
        m->count = count;
    }
    return ret;
}

int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m) {
    return cond_wait(c, m, NULL);
}

int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *abstime) {
    return cond_wait(c, m, abstime);
}

int pthread_cond_signal(pthread_cond_t *c) {
    __atomic_add_fetch(&c->seq, 1, __ATOMIC_SEQ_CST);
    futex_wake(&c->seq, 1);
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *c) {
    __atomic_add_fetch(&c->seq, 1, __ATOMIC_SEQ_CST);
    futex_wake(&c->seq, 0x7fffffff);
    return 0;
}

int pthread_condattr_init(pthread_condattr_t *attr) {
    attr->clock   = CLOCK_REALTIME;
    attr->pshared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock) {
    if (clock != CLOCK_REALTIME && clock != CLOCK_MONOTONIC)
        return EINVAL;
    attr->clock = clock;
    return 0;
}

int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock) {
    *clock = attr->clock;
    return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) {
    if (pshared != PTHREAD_PROCESS_PRIVATE)
        return ENOTSUP;
    attr->pshared = pshared;
    return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared) {
    *pshared = attr->pshared;
    return 0;
}
