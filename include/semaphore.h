#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>

typedef struct {
    volatile int value;
    volatile int waiters;
} sem_t;

#define SEM_FAILED  ((sem_t *)0)
#define SEM_VALUE_MAX 0x7fffffff

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *value);

// named semaphores need shared files, not there yet
sem_t *sem_open(const char *name, int flags, ...);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);

#ifdef __cplusplus
}
#endif
