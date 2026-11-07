#include <errno.h>
#include "thread/thread.h"

#undef errno

int __errno_main = 0;
extern int errno __attribute__((alias("__errno_main")));

int *__errno_location(void) {
    return thread_self()->errno_ptr;
}
