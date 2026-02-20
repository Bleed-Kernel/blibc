#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>

void perror(const char *s) {
    const char *msg = strerror(errno);

    if (s && *s) {
        (void)fprintf(stderr, "%s: %s\n", s, msg);
        return;
    }

    (void)fprintf(stderr, "%s\n", msg);
}
