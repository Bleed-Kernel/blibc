#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS     1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS           1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS          1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS           1

#include <stdarg.h>
#include <stddef.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>
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
            free(buf);
            return -1;
        }

        if ((size_t)written < size) {
            _write(fd, buf, written);
            free(buf);
            return written;
        }

        size = (size_t)written + 1;
        free(buf);
    }
}

int fprintf(int fd, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(fd, fmt, ap);
    va_end(ap);
    return ret;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return ret;
}