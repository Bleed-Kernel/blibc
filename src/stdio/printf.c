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
#include <libc/stdio.h>
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

typedef struct {
    int fd;
    size_t len;
    int failed;
    char buf[256];
} fd_putc_ctx_t;

static void fd_flush(fd_putc_ctx_t *ctx) {
    if (!ctx || ctx->failed || ctx->len == 0)
        return;

    if (write_all(ctx->fd, ctx->buf, ctx->len) < 0)
        ctx->failed = 1;

    ctx->len = 0;
}

static void fd_putc(int c, void *userdata) {
    fd_putc_ctx_t *ctx = (fd_putc_ctx_t *)userdata;
    if (!ctx || ctx->failed)
        return;

    ctx->buf[ctx->len++] = (char)c;
    if (ctx->len == sizeof(ctx->buf))
        fd_flush(ctx);
}

int vsprintf(char *str, const char *fmt, va_list ap) {
    return vsnprintf(str, (size_t)-1, fmt, ap);
}

int vdprintf(int fd, const char *fmt, va_list ap) {
    if (!fmt)
        return -1;

    fd_putc_ctx_t ctx = {
        .fd = fd,
        .len = 0,
        .failed = 0,
        .buf = {0}
    };

    int written = npf_vpprintf(fd_putc, &ctx, fmt, ap);
    fd_flush(&ctx);

    if (ctx.failed)
        return -1;

    return written;
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

int sprintf(char *str, const char *fmt, ...) {
    va_list ap;
    int rc;
    va_start(ap, fmt);
    rc = vsnprintf(str, (size_t)-1, fmt, ap);
    va_end(ap);
    return rc;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(stdout, fmt, ap);
    va_end(ap);
    return ret;
}

int vprintf(const char *fmt, va_list ap) {
    return vfprintf(stdout, fmt, ap);
}
