#pragma once
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <pthread.h>

#define SYS_SCHED_YIELD     24
#define SYS_GETTID          186
#define SYS_TKILL           200
#define SYS_FUTEX           202
#define SYS_SET_TID_ADDRESS 218
#define SYS_TGKILL          234

#define FUTEX_WAIT          0
#define FUTEX_WAKE          1
#define FUTEX_PRIVATE       128

#define THREAD_JOINABLE     0
#define THREAD_DETACHED     1
#define THREAD_EXITED       2

struct __pthread {
    struct __pthread *self;
    void        *dtv;
    void        *pad[3];
    uintptr_t   stack_guard;
    uintptr_t   pointer_guard;

    int         *errno_ptr;
    int         errno_val;
    volatile int tid;           // the kernel zeroes it and wakes joiners when the thread is gone
    volatile int state;

    void        *(*start)(void *);
    void        *arg;
    void        *result;

    void        *map;           // stack + tls + this, unmapped by whoever reaps the thread
    size_t      map_size;
    void        *stack_low;
    size_t      stack_size;

    struct __pthread_cleanup *cleanup;
    struct tls_dtor *tls_dtors;
    void        *locale;
    struct __pthread *next_thread;  // the live ones, tls.c keeps them
    struct __pthread *prev_thread;
    void        *tsd[PTHREAD_KEYS_MAX];
    int         tsd_used;
    char        name[16];
};

static inline struct __pthread *thread_self(void) {
    struct __pthread *self;
    __asm__ ("mov %%fs:0, %0" : "=r"(self));
    return self;
}

static inline long thread_syscall6(long n, long a, long b, long c, long d, long e, long f) {
    long ret;
    register long r10 __asm__("r10") = d;
    register long r8  __asm__("r8")  = e;
    register long r9  __asm__("r9")  = f;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9)
                      : "rcx", "r11", "memory");
    return ret;
}

static inline long futex_wait(volatile int *addr, int val, const struct timespec *rel) {
    return thread_syscall6(SYS_FUTEX, (long)addr, FUTEX_WAIT | FUTEX_PRIVATE, val, (long)rel, 0, 0);
}

static inline long futex_wake(volatile int *addr, int count) {
    return thread_syscall6(SYS_FUTEX, (long)addr, FUTEX_WAKE | FUTEX_PRIVATE, count, 0, 0, 0);
}

static inline void cpu_relax(void) {
    __asm__ volatile ("pause" ::: "memory");
}

// small futex lock for libc internals, 0 free 1 held 2 held with sleepers
static inline void lock_take(volatile int *l) {
    if (__sync_bool_compare_and_swap(l, 0, 1))
        return;
    for (int i = 0; i < 100; i++) {
        cpu_relax();
        if (*l == 0 && __sync_bool_compare_and_swap(l, 0, 1))
            return;
    }
    while (__atomic_exchange_n(l, 2, __ATOMIC_ACQUIRE) != 0)
        futex_wait(l, 2, NULL);
}

static inline void lock_give(volatile int *l) {
    if (__atomic_exchange_n(l, 0, __ATOMIC_RELEASE) == 2)
        futex_wake(l, 1);
}

// absolute time on clock to a relative timeout, -1 if its already passed
int __thread_rel_timeout(clockid_t clock, const struct timespec *abstime, struct timespec *rel);

// thread exit paths
void __thread_unmap_and_exit(void *map, size_t size) __attribute__((noreturn));
long __thread_clone(unsigned long flags, void *stack, volatile int *ptid, volatile int *ctid,
                    void *tls, int (*fn)(void *), void *arg);

// tls.c, a new threads tls block + descriptor at the top of mem
size_t __blibc_tls_size(void);
struct __pthread *__blibc_tls_setup(void *mem, size_t size);
void __blibc_tls_thread_add(struct __pthread *t);
void __blibc_tls_thread_remove(struct __pthread *t);
// the other threads didnt come through a fork
void __blibc_tls_fork_child(struct __pthread *self);


void __pthread_tsd_run(struct __pthread *self);
void __blibc_run_tls_dtors(void);

void __fork_prepare(void);
void __fork_parent(void);
void __fork_child(void);

extern volatile int __thread_count;
extern volatile int __malloc_lock;

extern int __errno_main;
