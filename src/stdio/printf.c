#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS     0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS          0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS           1

#include <stdarg.h>
#include <stddef.h>
#include <libc/stdlib.h>
#include <libc/syscalls/write.h>
#include <vendor/nanoprintf.h>

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    return npf_vsnprintf(buf, size, fmt, ap);
}

int vfprintf(int fd, const char *fmt, va_list ap) {
    char *buf = NULL;
    size_t size = 256;

    for (;;) {
        buf = malloc(size);
        if (!buf) return -1;

        va_list ap_copy;
        va_copy(ap_copy, ap);
        int written = npf_vsnprintf(buf, size, fmt, ap_copy);
        va_end(ap_copy);

        if (written < 0) {
            free(buf, size);
            return -1;
        }

        if ((size_t)written < size)
            break;

        free(buf, size);
        size <<= 1;
    }

    _write(fd, buf, size);
    free(buf, size);
    return 0;
}

int fprintf(int fd, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(fd, fmt, ap);
    va_end(ap);
    return ret;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(1, fmt, ap);
    va_end(ap);
    return ret;
}