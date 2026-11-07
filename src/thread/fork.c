#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include "thread.h"

typedef struct atfork {
    void (*prepare)(void);
    void (*parent)(void);
    void (*child)(void);
    struct atfork *next;
    struct atfork *prev;
} atfork_t;

static atfork_t *first;
static atfork_t *last;
static volatile int atfork_lock;

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
    atfork_t *a = malloc(sizeof(*a));
    if (!a)
        return ENOMEM;
    a->prepare = prepare;
    a->parent  = parent;
    a->child   = child;
    a->next    = NULL;

    lock_take(&atfork_lock);
    a->prev = last;
    if (last)
        last->next = a;
    else
        first = a;
    last = a;
    lock_give(&atfork_lock);
    return 0;
}

// prepare handlers run newest first, parent/child oldest first
void __fork_prepare(void) {
    lock_take(&atfork_lock);
    for (atfork_t *a = last; a; a = a->prev) {
        if (a->prepare)
            a->prepare();
    }
    // nobody else can be halfway through malloc when the child gets its copy
    lock_take(&__malloc_lock);
}

void __fork_parent(void) {
    lock_give(&__malloc_lock);
    for (atfork_t *a = first; a; a = a->next) {
        if (a->parent)
            a->parent();
    }
    lock_give(&atfork_lock);
}

void __fork_child(void) {
    struct __pthread *self = thread_self();
    __malloc_lock = 0;
    atfork_lock   = 0;
    __thread_count = 1;
    self->tid = (int)thread_syscall6(SYS_SET_TID_ADDRESS, (long)&self->tid, 0, 0, 0, 0, 0);
    __blibc_tls_fork_child(self);

    for (atfork_t *a = first; a; a = a->next) {
        if (a->child)
            a->child();
    }
}
