#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/random.h>
#include <sys/stat.h>

static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static int fill_name(char *tmpl, int suffixlen) {
    size_t len = strlen(tmpl);
    if (suffixlen < 0 || len < 6 + (size_t)suffixlen) {
        errno = EINVAL;
        return -1;
    }
    char *x = tmpl + len - suffixlen - 6;
    if (memcmp(x, "XXXXXX", 6) != 0) {
        errno = EINVAL;
        return -1;
    }
    unsigned char r[6];
    if (getrandom(r, sizeof(r), 0) != sizeof(r))
        return -1;
    for (int i = 0; i < 6; i++)
        x[i] = letters[r[i] % (sizeof(letters) - 1)];
    return 0;
}

int mkostemps(char *tmpl, int suffixlen, int flags) {
    for (int tries = 0; tries < 100; tries++) {
        if (fill_name(tmpl, suffixlen) < 0)
            return -1;
        int fd = open(tmpl, O_RDWR | O_CREAT | O_EXCL | (flags & ~O_ACCMODE), 0600);
        if (fd >= 0 || errno != EEXIST)
            return fd;
    }
    errno = EEXIST;
    return -1;
}

int mkstemp(char *tmpl)                         { return mkostemps(tmpl, 0, 0); }
int mkostemp(char *tmpl, int flags)             { return mkostemps(tmpl, 0, flags); }
int mkstemps(char *tmpl, int suffixlen)         { return mkostemps(tmpl, suffixlen, 0); }

char *mkdtemp(char *tmpl) {
    for (int tries = 0; tries < 100; tries++) {
        if (fill_name(tmpl, 0) < 0)
            return NULL;
        if (mkdir(tmpl, 0700) == 0)
            return tmpl;
        if (errno != EEXIST)
            return NULL;
    }
    errno = EEXIST;
    return NULL;
}

FILE *tmpfile(void) {
    char path[] = P_tmpdir "/tmpfileXXXXXX";
    int fd = mkstemp(path);
    if (fd < 0)
        return NULL;
    unlink(path);
    FILE *f = fdopen(fd, "w+");
    if (!f)
        close(fd);
    return f;
}
