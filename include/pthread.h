#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <sched.h>

typedef struct __pthread *pthread_t;
typedef unsigned int pthread_key_t;
typedef int pthread_once_t;
typedef volatile int pthread_spinlock_t;

typedef struct {
    size_t  stack_size;
    size_t  guard_size;
    void   *stack_addr;
    int     detached;
    int     sched_policy;
    int     sched_priority;
    int     inherit_sched;
    int     scope;
} pthread_attr_t;

typedef struct {
    volatile int lock;
    int     type;
    int     owner;
    int     count;
} pthread_mutex_t;

typedef struct {
    unsigned type     : 4;
    unsigned pshared  : 1;
    unsigned protocol : 2;
    unsigned robust   : 1;
} pthread_mutexattr_t;

typedef struct {
    volatile int seq;
    int     clock;
} pthread_cond_t;

typedef struct {
    unsigned clock   : 16;
    unsigned pshared : 1;
} pthread_condattr_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t  readers_ok;
    pthread_cond_t  writers_ok;
    int     readers;
    int     writer;
    int     writers_waiting;
} pthread_rwlock_t;

typedef struct {
    int     pshared;
} pthread_rwlockattr_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    unsigned int    count;
    unsigned int    waiting;
    unsigned int    generation;
} pthread_barrier_t;

typedef struct {
    int     pshared;
} pthread_barrierattr_t;

#define PTHREAD_MUTEX_NORMAL        0
#define PTHREAD_MUTEX_RECURSIVE     1
#define PTHREAD_MUTEX_ERRORCHECK    2
#define PTHREAD_MUTEX_DEFAULT       PTHREAD_MUTEX_NORMAL

#define PTHREAD_MUTEX_STALLED       0
#define PTHREAD_MUTEX_ROBUST        1

#define PTHREAD_PRIO_NONE           0
#define PTHREAD_PRIO_INHERIT        1
#define PTHREAD_PRIO_PROTECT        2

#define PTHREAD_CREATE_JOINABLE     0
#define PTHREAD_CREATE_DETACHED     1

#define PTHREAD_INHERIT_SCHED       0
#define PTHREAD_EXPLICIT_SCHED      1

#define PTHREAD_SCOPE_SYSTEM        0
#define PTHREAD_SCOPE_PROCESS       1

#define PTHREAD_PROCESS_PRIVATE     0
#define PTHREAD_PROCESS_SHARED      1

#define PTHREAD_CANCEL_ENABLE       0
#define PTHREAD_CANCEL_DISABLE      1
#define PTHREAD_CANCEL_DEFERRED     0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1
#define PTHREAD_CANCELED            ((void *)-1)

#define PTHREAD_BARRIER_SERIAL_THREAD -1

#define PTHREAD_KEYS_MAX            128
#define PTHREAD_DESTRUCTOR_ITERATIONS 4
#define PTHREAD_STACK_MIN           16384

#define PTHREAD_ONCE_INIT           0
#define PTHREAD_MUTEX_INITIALIZER   { 0, PTHREAD_MUTEX_NORMAL, 0, 0 }
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP { 0, PTHREAD_MUTEX_RECURSIVE, 0, 0 }
#define PTHREAD_COND_INITIALIZER    { 0, CLOCK_REALTIME }
#define PTHREAD_RWLOCK_INITIALIZER  { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, \
                                      PTHREAD_COND_INITIALIZER, 0, 0, 0 }

// threads
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
int pthread_join(pthread_t thread, void **result);
int pthread_detach(pthread_t thread);
void pthread_exit(void *result) __attribute__((noreturn));
pthread_t pthread_self(void);
int pthread_equal(pthread_t a, pthread_t b);
int pthread_once(pthread_once_t *once, void (*fn)(void));
int pthread_kill(pthread_t thread, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
int pthread_setname_np(pthread_t thread, const char *name);
int pthread_getname_np(pthread_t thread, char *name, size_t len);
int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr);
pid_t pthread_gettid_np(pthread_t thread);
int pthread_getconcurrency(void);
int pthread_setconcurrency(int level);
int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
int pthread_setschedprio(pthread_t thread, int prio);
int pthread_getcpuclockid(pthread_t thread, clockid_t *clock);

// cancellation isnt supported, these are here so things compile
int pthread_cancel(pthread_t thread);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
void pthread_testcancel(void);

// cleanup handlers, run by pthread_exit
struct __pthread_cleanup {
    void (*fn)(void *);
    void *arg;
    struct __pthread_cleanup *next;
};
void __pthread_cleanup_push(struct __pthread_cleanup *c, void (*fn)(void *), void *arg);
void __pthread_cleanup_pop(struct __pthread_cleanup *c, int run);
#define pthread_cleanup_push(fn, arg) do { struct __pthread_cleanup __cb; __pthread_cleanup_push(&__cb, fn, arg);
#define pthread_cleanup_pop(run) __pthread_cleanup_pop(&__cb, run); } while (0)

// attributes
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int state);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size);
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size);
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t size);
int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *size);
int pthread_attr_setstack(pthread_attr_t *attr, void *addr, size_t size);
int pthread_attr_getstack(const pthread_attr_t *attr, void **addr, size_t *size);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *addr);
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **addr);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit);
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit);
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope);

// mutexes
int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *m);
int pthread_mutex_lock(pthread_mutex_t *m);
int pthread_mutex_trylock(pthread_mutex_t *m);
int pthread_mutex_timedlock(pthread_mutex_t *m, const struct timespec *abstime);
int pthread_mutex_unlock(pthread_mutex_t *m);
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared);
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol);
int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust);
int pthread_mutexattr_getrobust(const pthread_mutexattr_t *attr, int *robust);

// condition variables
int pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *c);
int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *c);
int pthread_cond_broadcast(pthread_cond_t *c);
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock);
int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock);
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared);

// read write locks
int pthread_rwlock_init(pthread_rwlock_t *rw, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rw);
int pthread_rwlock_rdlock(pthread_rwlock_t *rw);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rw);
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rw, const struct timespec *abstime);
int pthread_rwlock_wrlock(pthread_rwlock_t *rw);
int pthread_rwlock_trywrlock(pthread_rwlock_t *rw);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rw, const struct timespec *abstime);
int pthread_rwlock_unlock(pthread_rwlock_t *rw);
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared);

// spinlocks and barriers
int pthread_spin_init(pthread_spinlock_t *s, int pshared);
int pthread_spin_destroy(pthread_spinlock_t *s);
int pthread_spin_lock(pthread_spinlock_t *s);
int pthread_spin_trylock(pthread_spinlock_t *s);
int pthread_spin_unlock(pthread_spinlock_t *s);
int pthread_barrier_init(pthread_barrier_t *b, const pthread_barrierattr_t *attr, unsigned int count);
int pthread_barrier_destroy(pthread_barrier_t *b);
int pthread_barrier_wait(pthread_barrier_t *b);
int pthread_barrierattr_init(pthread_barrierattr_t *attr);
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared);

// thread specific data
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

// one affinity for the whole process like sched_setaffinity
#include <sched.h>
int pthread_setaffinity_np(pthread_t thread, size_t size, const cpu_set_t *set);
int pthread_getaffinity_np(pthread_t thread, size_t size, cpu_set_t *set);

#ifdef __cplusplus
}
#endif
