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
#include <stdint.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/devices/console.h>
#include <libc/syscalls/ioctl.h>
#include <libc/syscalls/write.h>
#include <vendor/nanoprintf.h>

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    return npf_vsnprintf(buf, size, fmt, ap);
}

static int write_all(int fd, const char *buf, size_t len) {
    size_t off = 0;

    while (off < len) {
        long w = _write(fd, buf + off, (long)(len - off));
        if (w <= 0) return -1;
        off += (size_t)w;
    }

    return 0;
}

static int parse_u32(const char *s, size_t len, uint32_t *out, int *present) {
    size_t i = 0;
    uint32_t v = 0;

    *present = 0;
    if (len == 0) {
        *out = 0;
        return 0;
    }

    for (i = 0; i < len; ++i) {
        char c = s[i];
        if (c < '0' || c > '9') return -1;
        *present = 1;
        v = v * 10u + (uint32_t)(c - '0');
    }

    *out = v;
    return 0;
}

static int tty_apply_csi_cursor(int fd, const char *params, size_t plen, char final) {
    uint32_t a = 0, b = 0;
    int has_a = 0, has_b = 0;
    tty_cursor_t cur;
    size_t sep = plen;

    if (_ioctl(fd, TTY_IOCTL_GET_CURSOR, &cur) < 0) return -1;

    for (size_t i = 0; i < plen; ++i) {
        if (params[i] == ';') {
            sep = i;
            break;
        }
    }

    if (parse_u32(params, sep, &a, &has_a) < 0) return -1;
    if (sep < plen) {
        if (parse_u32(params + sep + 1, plen - sep - 1, &b, &has_b) < 0) return -1;
    }

    switch (final) {
    case 'A': {
        uint32_t n = (!has_a || a == 0) ? 1u : a;
        cur.y = (cur.y > n) ? (cur.y - n) : 0;
        break;
    }
    case 'B': {
        uint32_t n = (!has_a || a == 0) ? 1u : a;
        cur.y += n;
        break;
    }
    case 'C': {
        uint32_t n = (!has_a || a == 0) ? 1u : a;
        cur.x += n;
        break;
    }
    case 'D': {
        uint32_t n = (!has_a || a == 0) ? 1u : a;
        cur.x = (cur.x > n) ? (cur.x - n) : 0;
        break;
    }
    case 'H':
    case 'f': {
        uint32_t row = (!has_a || a == 0) ? 1u : a;
        uint32_t col = (!has_b || b == 0) ? 1u : b;
        cur.y = row - 1u;
        cur.x = col - 1u;
        break;
    }
    case 'G': {
        uint32_t col = (!has_a || a == 0) ? 1u : a;
        cur.x = col - 1u;
        break;
    }
    default:
        return -1;
    }

    return _ioctl(fd, TTY_IOCTL_SET_CURSOR, &cur);
}

static int write_with_tty_cursor_compat(int fd, const char *buf, size_t len) {
    tty_cursor_t probe;
    size_t plain_start = 0;
    size_t i = 0;

    if (_ioctl(fd, TTY_IOCTL_GET_CURSOR, &probe) < 0) {
        return write_all(fd, buf, len);
    }

    while (i < len) {
        if (buf[i] == '\x1b' && (i + 2) < len && buf[i + 1] == '[') {
            size_t j = i + 2;
            while (j < len) {
                unsigned char c = (unsigned char)buf[j];
                if (c >= 0x40u && c <= 0x7eu) break;
                ++j;
            }

            if (j < len) {
                char final = buf[j];
                size_t plen = j - (i + 2);
                if (tty_apply_csi_cursor(fd, buf + i + 2, plen, final) == 0) {
                    if (plain_start < i && write_all(fd, buf + plain_start, i - plain_start) < 0) {
                        return -1;
                    }
                    i = j + 1;
                    plain_start = i;
                    continue;
                }
            }
        }
        ++i;
    }

    if (plain_start < len) {
        return write_all(fd, buf + plain_start, len - plain_start);
    }
    return 0;
}

int vdprintf(int fd, const char *fmt, va_list ap) {
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
            if (write_with_tty_cursor_compat(fd, buf, (size_t)written) < 0) {
                free(buf);
                return -1;
            }
            free(buf);
            return written;
        }

        size = (size_t)written + 1;
        free(buf);
    }
}

int dprintf(int fd, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vdprintf(fd, fmt, ap);
    va_end(ap);
    return ret;
}

int vfprintf(FILE *stream, const char *fmt, va_list ap) {
    int ret;

    if (!stream) {
        return -1;
    }

    ret = vdprintf(stream->fd, fmt, ap);
    if (ret < 0) {
        stream->error = 1;
    }
    return ret;
}

int fprintf(FILE *stream, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(stream, fmt, ap);
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