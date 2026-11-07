#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../thread/thread.h"

int __blibc_atexit_arg(void (*fn)(void *), void *arg);

// normally from crtbegin.o, which bleed programs dont link
__attribute__((weak, visibility("hidden"))) void *__dso_handle = &__dso_handle;

// static object destructors, one program per address space so the dso doesnt matter
int __cxa_atexit(void (*fn)(void *), void *arg, void *dso) {
    (void)dso;
    return __blibc_atexit_arg(fn, arg);
}

// only dlclose would want this and there isnt one
void __cxa_finalize(void *dso) {
    (void)dso;
}

__attribute__((weak)) void __cxa_pure_virtual(void) {
    fprintf(stderr, "pure virtual function called\n");
    abort();
}

__attribute__((weak)) void __cxa_deleted_virtual(void) {
    fprintf(stderr, "deleted virtual function called\n");
    abort();
}

#define GUARD_DONE      0x1
#define GUARD_BUSY      0x100
#define GUARD_WAITING   0x10000

__attribute__((weak)) int __cxa_guard_acquire(uint64_t *guard) {
    volatile int *word = (volatile int *)guard;

    for (;;) {
        int seen = __atomic_load_n(word, __ATOMIC_ACQUIRE);
        if (seen & GUARD_DONE)
            return 0;
        if (!(seen & GUARD_BUSY)) {
            if (__sync_bool_compare_and_swap(word, seen, seen | GUARD_BUSY))
                return 1;
            continue;
        }
        seen = __atomic_fetch_or(word, GUARD_WAITING, __ATOMIC_ACQ_REL) | GUARD_WAITING;
        if ((seen & GUARD_BUSY) && !(seen & GUARD_DONE))
            futex_wait(word, seen, NULL);
    }
}

static void guard_finish(uint64_t *guard, int done) {
    volatile int *word = (volatile int *)guard;
    int old = __atomic_exchange_n(word, done ? GUARD_DONE : 0, __ATOMIC_ACQ_REL);
    if (old & GUARD_WAITING)
        futex_wake(word, 0x7fffffff);
}

__attribute__((weak)) void __cxa_guard_release(uint64_t *guard) {
    guard_finish(guard, 1);
}

__attribute__((weak)) void __cxa_guard_abort(uint64_t *guard) {
    guard_finish(guard, 0);
}
