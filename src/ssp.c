#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uintptr_t __stack_chk_guard = 0x595e9fbd94fda766ULL;

void __stack_chk_fail(void) {
    fprintf(stderr, "stack smashing detected\n");
    abort();
}
