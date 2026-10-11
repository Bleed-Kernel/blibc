#include <errno.h>
#include <stdio.h>
#include <string.h>

void perror(const char *s) {
    const char *msg = strerror(errno);

    if (s && *s) {
        (void)fprintf(stderr, "%s: %s\n", s, msg);
        return;
    }

    (void)fprintf(stderr, "%s\n", msg);
}
