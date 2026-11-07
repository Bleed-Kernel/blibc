#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdarg.h>
#include "thread.h"

int pthread_spin_init(pthread_spinlock_t *s, int pshared) {
    (void)pshared;
    *s = 0;
    return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *s) {
    (void)s;
    return 0;
}

int pthread_spin_lock(pthread_spinlock_t *s) {
    while (__atomic_exchange_n(s, 1, __ATOMIC_ACQUIRE)) {
        while (*s)
            cpu_relax();
    }
    return 0;
}

int pthread_spin_trylock(pthread_spinlock_t *s) {
    return __atomic_exchange_n(s, 1, __ATOMIC_ACQUIRE) ? EBUSY : 0;
}

int pthread_spin_unlock(pthread_spinlock_t *s) {
    __atomic_store_n(s, 0, __ATOMIC_RELEASE);
    return 0;
}

int pthread_barrier_init(pthread_barrier_t *b, const pthread_barrierattr_t *attr, unsigned int count) {
    (void)attr;
    if (!count)
        return EINVAL;
    pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->cond, NULL);
    b->count      = count;
    b->waiting    = 0;
    b->generation = 0;
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *b) {
    return b->waiting ? EBUSY : 0;
}

int pthread_barrier_wait(pthread_barrier_t *b) {
    pthread_mutex_lock(&b->lock);
    unsigned int gen = b->generation;
    if (++b->waiting == b->count) {
        b->waiting = 0;
        b->generation++;
        pthread_cond_broadcast(&b->cond);
        pthread_mutex_unlock(&b->lock);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    }
    while (gen == b->generation)
        pthread_cond_wait(&b->cond, &b->lock);
    pthread_mutex_unlock(&b->lock);
    return 0;
}

int pthread_barrierattr_init(pthread_barrierattr_t *attr) {
    attr->pshared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared) {
    if (pshared != PTHREAD_PROCESS_PRIVATE)
        return ENOTSUP;
    attr->pshared = pshared;
    return 0;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared) {
    *pshared = attr->pshared;
    return 0;
}

// semaphores
int sem_init(sem_t *sem, int pshared, unsigned int value) {
    if (value > SEM_VALUE_MAX) {
        errno = EINVAL;
        return -1;
    }
    if (pshared) {
        errno = ENOSYS;
        return -1;
    }
    sem->value   = (int)value;
    sem->waiters = 0;
    return 0;
}

int sem_destroy(sem_t *sem) {
    (void)sem;
    return 0;
}

int sem_trywait(sem_t *sem) {
    int v;
    while ((v = sem->value) > 0) {
        if (__sync_bool_compare_and_swap(&sem->value, v, v - 1))
            return 0;
    }
    errno = EAGAIN;
    return -1;
}

static int sem_sleep(sem_t *sem, const struct timespec *abstime) {
    for (;;) {
        if (sem_trywait(sem) == 0)
            return 0;

        struct timespec rel;
        if (abstime) {
            if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000) {
                errno = EINVAL;
                return -1;
            }
            if (__thread_rel_timeout(CLOCK_REALTIME, abstime, &rel) < 0) {
                errno = ETIMEDOUT;
                return -1;
            }
        }

        __atomic_add_fetch(&sem->waiters, 1, __ATOMIC_SEQ_CST);
        long r = futex_wait(&sem->value, 0, abstime ? &rel : NULL);
        __atomic_sub_fetch(&sem->waiters, 1, __ATOMIC_SEQ_CST);
        if (r == -EINTR) {
            errno = EINTR;
            return -1;
        }
    }
}

int sem_wait(sem_t *sem) {
    return sem_sleep(sem, NULL);
}

int sem_timedwait(sem_t *sem, const struct timespec *abstime) {
    return sem_sleep(sem, abstime);
}

int sem_post(sem_t *sem) {
    if (sem->value == SEM_VALUE_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    __atomic_add_fetch(&sem->value, 1, __ATOMIC_SEQ_CST);
    if (sem->waiters)
        futex_wake(&sem->value, 1);
    return 0;
}

int sem_getvalue(sem_t *sem, int *value) {
    *value = sem->value;
    return 0;
}

sem_t *sem_open(const char *name, int flags, ...) {
    (void)name;
    (void)flags;
    errno = ENOSYS;
    return SEM_FAILED;
}

int sem_close(sem_t *sem) {
    (void)sem;
    errno = ENOSYS;
    return -1;
}

int sem_unlink(const char *name) {
    (void)name;
    errno = ENOSYS;
    return -1;
}
