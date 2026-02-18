#include <libc/errno.h>
#include <libc/stdlib.h>

// stub
int system(const char *command) {
    if (!command) {
        return 0;
    }

    errno = ENOSYS;
    return -1;
}
