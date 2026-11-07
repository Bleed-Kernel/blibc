#include <pthread.h>
#include <errno.h>
#include "thread.h"

int pthread_rwlock_init(pthread_rwlock_t *rw, const pthread_rwlockattr_t *attr) {
    (void)attr;
    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->readers_ok, NULL);
    pthread_cond_init(&rw->writers_ok, NULL);
    rw->readers = 0;
    rw->writer = 0;
    rw->writers_waiting = 0;
    return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rw) {
    return rw->readers || rw->writer ? EBUSY : 0;
}

static int rdlock(pthread_rwlock_t *rw, int try, const struct timespec *abstime) {
    int ret = 0;
    pthread_mutex_lock(&rw->lock);
    while (rw->writer) {
        if (try) {
            ret = EBUSY;
            break;
        }
        if (abstime)
            ret = pthread_cond_timedwait(&rw->readers_ok, &rw->lock, abstime);
        else
            ret = pthread_cond_wait(&rw->readers_ok, &rw->lock);
        if (ret)
            break;
    }
    if (!ret)
        rw->readers++;
    pthread_mutex_unlock(&rw->lock);
    return ret;
}

static int wrlock(pthread_rwlock_t *rw, int try, const struct timespec *abstime) {
    int ret = 0;
    pthread_mutex_lock(&rw->lock);
    if (rw->writer == thread_self()->tid) {
        pthread_mutex_unlock(&rw->lock);
        return EDEADLK;
    }
    rw->writers_waiting++;
    while (rw->writer || rw->readers) {
        if (try) {
            ret = EBUSY;
            break;
        }
        if (abstime)
            ret = pthread_cond_timedwait(&rw->writers_ok, &rw->lock, abstime);
        else
            ret = pthread_cond_wait(&rw->writers_ok, &rw->lock);
        if (ret)
            break;
    }
    rw->writers_waiting--;
    if (!ret)
        rw->writer = thread_self()->tid;
    pthread_mutex_unlock(&rw->lock);
    return ret;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rw) {
    return rdlock(rw, 0, NULL);
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rw) {
    return rdlock(rw, 1, NULL);
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rw, const struct timespec *abstime) {
    return rdlock(rw, 0, abstime);
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rw) {
    return wrlock(rw, 0, NULL);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rw) {
    return wrlock(rw, 1, NULL);
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *rw, const struct timespec *abstime) {
    return wrlock(rw, 0, abstime);
}

int pthread_rwlock_unlock(pthread_rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    if (rw->writer)
        rw->writer = 0;
    else if (rw->readers)
        rw->readers--;

    if (!rw->readers && rw->writers_waiting)
        pthread_cond_signal(&rw->writers_ok);
    if (!rw->writer)
        pthread_cond_broadcast(&rw->readers_ok);
    pthread_mutex_unlock(&rw->lock);
    return 0;
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr) {
    attr->pshared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared) {
    if (pshared != PTHREAD_PROCESS_PRIVATE)
        return ENOTSUP;
    attr->pshared = pshared;
    return 0;
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared) {
    *pshared = attr->pshared;
    return 0;
}
