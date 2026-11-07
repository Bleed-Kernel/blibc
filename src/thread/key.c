#include <pthread.h>
#include <errno.h>
#include "thread.h"
#include <stdlib.h>

typedef struct {
    void    (*destructor)(void *);
    int     used;
} key_slot_t;

static key_slot_t keys[PTHREAD_KEYS_MAX];
static volatile int keys_lock;

static void no_destructor(void *p) {
    (void)p;
}

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
    lock_take(&keys_lock);
    for (unsigned int i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if (!keys[i].used) {
            keys[i].used = 1;
            keys[i].destructor = destructor ? destructor : no_destructor;
            lock_give(&keys_lock);
            *key = i;
            return 0;
        }
    }
    lock_give(&keys_lock);
    return EAGAIN;
}

// other threads values are left, like posix says, its on the caller to have cleaned them
int pthread_key_delete(pthread_key_t key) {
    if (key >= PTHREAD_KEYS_MAX)
        return EINVAL;
    lock_take(&keys_lock);
    keys[key].used = 0;
    lock_give(&keys_lock);
    thread_self()->tsd[key] = NULL;
    return 0;
}

void *pthread_getspecific(pthread_key_t key) {
    if (key >= PTHREAD_KEYS_MAX)
        return NULL;
    return thread_self()->tsd[key];
}

int pthread_setspecific(pthread_key_t key, const void *value) {
    if (key >= PTHREAD_KEYS_MAX || !keys[key].used)
        return EINVAL;
    struct __pthread *self = thread_self();
    self->tsd[key] = (void *)value;
    if (value)
        self->tsd_used = 1;
    return 0;
}

void __pthread_tsd_run(struct __pthread *self) {
    for (int pass = 0; pass < PTHREAD_DESTRUCTOR_ITERATIONS && self->tsd_used; pass++) {
        self->tsd_used = 0;
        for (int i = 0; i < PTHREAD_KEYS_MAX; i++) {
            void *v = self->tsd[i];
            void (*dtor)(void *) = keys[i].destructor;
            if (!v || !keys[i].used)
                continue;
            self->tsd[i] = NULL;
            dtor(v);
        }
    }
}

// c++ thread_local destructors, run newest first when the thread ends
struct tls_dtor {
    void (*fn)(void *);
    void *obj;
    struct tls_dtor *next;
};

int __cxa_thread_atexit(void (*fn)(void *), void *obj, void *dso) {
    (void)dso;
    struct tls_dtor *d = malloc(sizeof(*d));
    if (!d)
        return -1;
    struct __pthread *self = thread_self();
    d->fn   = fn;
    d->obj  = obj;
    d->next = self->tls_dtors;
    self->tls_dtors = d;
    return 0;
}

int __cxa_thread_atexit_impl(void (*fn)(void *), void *obj, void *dso) {
    return __cxa_thread_atexit(fn, obj, dso);
}

void __blibc_run_tls_dtors(void) {
    struct __pthread *self = thread_self();
    while (self->tls_dtors) {
        struct tls_dtor *d = self->tls_dtors;
        self->tls_dtors = d->next;
        d->fn(d->obj);
        free(d);
    }
}
