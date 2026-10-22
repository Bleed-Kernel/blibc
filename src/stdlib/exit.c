#include <stdlib.h>
#include <bleed/syscalls.h>

#define ATEXIT_MAX  32

static void (*handlers[ATEXIT_MAX])(void);
static int  handler_count;

int atexit(void (*fn)(void)) {
    if (!fn || handler_count == ATEXIT_MAX)
        return -1;
    handlers[handler_count++] = fn;
    return 0;
}

// handlers run last registered first
__attribute__((noreturn))
void exit(int code){
    while (handler_count > 0)
        handlers[--handler_count]();
    _exit(code);
    __builtin_unreachable();
}

__attribute__((noreturn))
void _Exit(int code) {
    _exit(code);
    __builtin_unreachable();
}
