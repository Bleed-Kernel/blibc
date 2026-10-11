#include <stdio.h>
#include <stdlib.h>
#include <bleed/syscalls.h>

void abort(void) {
    _exit(134);
    __builtin_unreachable();
}

void __assert_fail(const char *expr, const char *file, int line, const char *func) {
    fprintf(stderr, "assertion failed: %s (%s:%d in %s)\n", expr, file, line, func ? func : "?");
    abort();
}
