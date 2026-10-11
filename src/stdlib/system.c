#include <errno.h>
#include <stdlib.h>

// stub
int system(const char *command) {
    if (!command) {
        return 0;
    }

    errno = ENOSYS;
    return -1;
}
