#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/unistd.h>

static int write_all(int fd, const char *buf, size_t len) {
    size_t off = 0;

    while (off < len) {
        ssize_t w = write(fd, buf + off, len - off);
        if (w <= 0) {
            return -1;
        }
        off += (size_t)w;
    }

    return 0;
}

int putchar(int c) {
    char ch = (char)c;
    if (write_all(STDOUT_FILENO, &ch, 1) < 0) {
        return -1;
    }
    return (unsigned char)ch;
}

int puts(const char *s) {
    if (!s) {
        return -1;
    }

    size_t len = strlen(s);
    if (write_all(STDOUT_FILENO, s, len) < 0) {
        return -1;
    }
    if (write_all(STDOUT_FILENO, "\n", 1) < 0) {
        return -1;
    }

    return (int)(len + 1);
}
