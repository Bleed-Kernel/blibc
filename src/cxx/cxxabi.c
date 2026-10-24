#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

void __cxa_pure_virtual(void) {
    fprintf(stderr, "pure virtual function called\n");
    abort();
}

void __cxa_deleted_virtual(void) {
    fprintf(stderr, "deleted virtual function called\n");
    abort();
}

int __cxa_guard_acquire(uint64_t *guard) {
    uint8_t *g = (uint8_t *)guard;
    if (g[0])
        return 0;
    if (g[1]) {
        fprintf(stderr, "static initialised recursively\n");
        abort();
    }
    g[1] = 1;
    return 1;
}

void __cxa_guard_release(uint64_t *guard) {
    uint8_t *g = (uint8_t *)guard;
    g[0] = 1;
    g[1] = 0;
}

void __cxa_guard_abort(uint64_t *guard) {
    ((uint8_t *)guard)[1] = 0;
}
