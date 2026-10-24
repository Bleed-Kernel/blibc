#include <stdlib.h>
#include <bleed/syscalls.h>

#define ATEXIT_BLOCK  32

// atexit and c++ static destructors (__cxa_atexit) share one list so they unwind in order
typedef struct {
    void (*fn)(void);
    void (*arg_fn)(void *);
    void *arg;
} exit_handler_t;

typedef struct exit_block {
    struct exit_block *prev;
    int count;
    exit_handler_t handlers[ATEXIT_BLOCK];
} exit_block_t;

static exit_block_t first_block;
static exit_block_t *top = &first_block;

typedef void (*array_fn_t)(void);

extern array_fn_t __preinit_array_start[] __attribute__((weak));
extern array_fn_t __preinit_array_end[] __attribute__((weak));
extern array_fn_t __init_array_start[] __attribute__((weak));
extern array_fn_t __init_array_end[] __attribute__((weak));
extern array_fn_t __fini_array_start[] __attribute__((weak));
extern array_fn_t __fini_array_end[] __attribute__((weak));

static exit_handler_t *new_handler(void) {
    if (top->count == ATEXIT_BLOCK) {
        exit_block_t *b = calloc(1, sizeof(*b));
        if (!b)
            return NULL;
        b->prev = top;
        top = b;
    }
    return &top->handlers[top->count++];
}

int atexit(void (*fn)(void)) {
    exit_handler_t *h;
    if (!fn || !(h = new_handler()))
        return -1;
    *h = (exit_handler_t){ .fn = fn };
    return 0;
}

int __blibc_atexit_arg(void (*fn)(void *), void *arg) {
    exit_handler_t *h;
    if (!fn || !(h = new_handler()))
        return -1;
    *h = (exit_handler_t){ .arg_fn = fn, .arg = arg };
    return 0;
}

// constructors, c ones and c++ globals (g++ puts those in .ctors, ld folds that into .init_array)
void __blibc_init_array(void) {
    for (array_fn_t *f = __preinit_array_start; f < __preinit_array_end; f++)
        (*f)();
    for (array_fn_t *f = __init_array_start; f < __init_array_end; f++)
        (*f)();
}

// handlers run last registered first, then the destructors
__attribute__((noreturn))
void exit(int code){
    for (;;) {
        while (top->count > 0) {
            exit_handler_t h = top->handlers[--top->count];
            if (h.fn)
                h.fn();
            else
                h.arg_fn(h.arg);
        }
        if (!top->prev)
            break;
        top = top->prev;
    }

    for (array_fn_t *f = __fini_array_end; f > __fini_array_start; )
        (*--f)();

    _exit(code);
    __builtin_unreachable();
}

__attribute__((noreturn))
void _Exit(int code) {
    _exit(code);
    __builtin_unreachable();
}
