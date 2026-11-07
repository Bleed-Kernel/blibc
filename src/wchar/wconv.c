#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#define NUM_MAX 4096

static char *ascii_prefix(const wchar_t *s, char *small, size_t small_len) {
    size_t n = 0;
    while (s[n] && s[n] < 0x80 && n < NUM_MAX)
        n++;
    char *buf = n < small_len ? small : malloc(n + 1);
    if (!buf)
        return NULL;
    for (size_t i = 0; i < n; i++)
        buf[i] = (char)s[i];
    buf[n] = 0;
    return buf;
}

#define WIDE_TO_NUM(name, type, call)                                   \
    type name {                                                         \
        char small[128];                                                \
        char *buf = ascii_prefix(s, small, sizeof(small));             \
        if (!buf) {                                                     \
            if (end)                                                    \
                *end = (wchar_t *)s;                                    \
            return 0;                                                   \
        }                                                               \
        char *stop;                                                     \
        type v = call;                                                  \
        if (end)                                                        \
            *end = (wchar_t *)s + (stop - buf);                         \
        if (buf != small)                                               \
            free(buf);                                                  \
        return v;                                                       \
    }

WIDE_TO_NUM(wcstol(const wchar_t *s, wchar_t **end, int base), long, strtol(buf, &stop, base))
WIDE_TO_NUM(wcstoul(const wchar_t *s, wchar_t **end, int base), unsigned long, strtoul(buf, &stop, base))
WIDE_TO_NUM(wcstoll(const wchar_t *s, wchar_t **end, int base), long long, strtoll(buf, &stop, base))
WIDE_TO_NUM(wcstoull(const wchar_t *s, wchar_t **end, int base), unsigned long long, strtoull(buf, &stop, base))
WIDE_TO_NUM(wcstod(const wchar_t *s, wchar_t **end), double, strtod(buf, &stop))
WIDE_TO_NUM(wcstof(const wchar_t *s, wchar_t **end), float, strtof(buf, &stop))
WIDE_TO_NUM(wcstold(const wchar_t *s, wchar_t **end), long double, strtold(buf, &stop))

// wide to a malloced utf-8 copy and back
static char *narrow(const wchar_t *w) {
    mbstate_t st = { 0 };
    const wchar_t *p = w;
    size_t n = wcsrtombs(NULL, &p, 0, &st);
    if (n == (size_t)-1)
        return NULL;
    char *s = malloc(n + 1);
    if (!s)
        return NULL;
    p = w;
    memset(&st, 0, sizeof(st));
    wcsrtombs(s, &p, n + 1, &st);
    s[n] = 0;
    return s;
}

static int widen_into(wchar_t *dst, size_t n, const char *s) {
    mbstate_t st = { 0 };
    const char *p = s;
    size_t need = mbsrtowcs(NULL, &p, 0, &st);
    if (need == (size_t)-1 || !n)
        return -1;
    p = s;
    memset(&st, 0, sizeof(st));
    size_t got = mbsrtowcs(dst, &p, n, &st);
    if (got >= n || need >= n) {
        dst[n - 1] = 0;
        return -1;
    }
    dst[got] = 0;
    return (int)got;
}

size_t wcsftime(wchar_t *s, size_t max, const wchar_t *fmt, const struct tm *tm) {
    char *f = narrow(fmt);
    if (!f)
        return 0;
    char out[512];
    size_t n = strftime(out, sizeof(out), f, tm);
    free(f);
    if (!n && *fmt)
        return 0;
    int got = widen_into(s, max, out);
    return got < 0 ? 0 : (size_t)got;
}

// the printf family through the narrow one
int vswprintf(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap) {
    char *f = narrow(fmt);
    if (!f)
        return -1;
    char *out;
    int r = vasprintf(&out, f, ap);
    free(f);
    if (r < 0)
        return -1;
    int got = widen_into(s, n, out);
    free(out);
    return got;
}

int swprintf(wchar_t *s, size_t n, const wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vswprintf(s, n, fmt, ap);
    va_end(ap);
    return r;
}

int vfwprintf(FILE *f, const wchar_t *fmt, va_list ap) {
    char *nf = narrow(fmt);
    if (!nf)
        return -1;
    char *out;
    int r = vasprintf(&out, nf, ap);
    free(nf);
    if (r < 0)
        return -1;
    fputs(out, f);
    mbstate_t st = { 0 };
    const char *p = out;
    size_t wide = mbsrtowcs(NULL, &p, 0, &st);
    free(out);
    return wide == (size_t)-1 ? -1 : (int)wide;
}

int fwprintf(FILE *f, const wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfwprintf(f, fmt, ap);
    va_end(ap);
    return r;
}

int vwprintf(const wchar_t *fmt, va_list ap) {
    return vfwprintf(stdout, fmt, ap);
}

int wprintf(const wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfwprintf(stdout, fmt, ap);
    va_end(ap);
    return r;
}

// numbers and narrow strings only, %ls lands as utf-8 bytes
int vswscanf(const wchar_t *s, const wchar_t *fmt, va_list ap) {
    char *ns = narrow(s);
    char *nf = narrow(fmt);
    int   r  = EOF;
    if (ns && nf)
        r = vsscanf(ns, nf, ap);
    free(ns);
    free(nf);
    return r;
}

int swscanf(const wchar_t *s, const wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vswscanf(s, fmt, ap);
    va_end(ap);
    return r;
}

// TODO: reading wide input straight off a stream
int vfwscanf(FILE *f, const wchar_t *fmt, va_list ap) {
    (void)f;
    (void)fmt;
    (void)ap;
    errno = ENOSYS;
    return EOF;
}

int fwscanf(FILE *f, const wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfwscanf(f, fmt, ap);
    va_end(ap);
    return r;
}

int vwscanf(const wchar_t *fmt, va_list ap) {
    return vfwscanf(stdin, fmt, ap);
}

int wscanf(const wchar_t *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfwscanf(stdin, fmt, ap);
    va_end(ap);
    return r;
}

// wide chars on byte streams, utf-8 on the wire
wint_t fgetwc(FILE *f) {
    mbstate_t st = { 0 };
    for (int i = 0; i < 4; i++) {
        int c = fgetc(f);
        if (c == EOF)
            return WEOF;
        char    b  = (char)c;
        wchar_t wc;
        size_t  r  = mbrtowc(&wc, &b, 1, &st);
        if (r == (size_t)-1)
            return WEOF;
        if (r != (size_t)-2)
            return (wint_t)wc;
    }
    errno = EILSEQ;
    return WEOF;
}

wint_t getwc(FILE *f) {
    return fgetwc(f);
}

wint_t getwchar(void) {
    return fgetwc(stdin);
}

wint_t fputwc(wchar_t c, FILE *f) {
    char   buf[4];
    size_t n = wcrtomb(buf, c, NULL);
    if (n == (size_t)-1 || fwrite(buf, 1, n, f) != n)
        return WEOF;
    return (wint_t)c;
}

wint_t putwc(wchar_t c, FILE *f) {
    return fputwc(c, f);
}

wint_t putwchar(wchar_t c) {
    return fputwc(c, stdout);
}

wint_t ungetwc(wint_t c, FILE *f) {
    if (c == WEOF)
        return WEOF;
    char   buf[4];
    size_t n = wcrtomb(buf, (wchar_t)c, NULL);
    if (n == (size_t)-1)
        return WEOF;
    for (size_t i = n; i; i--) {
        if (ungetc((unsigned char)buf[i - 1], f) == EOF)
            return WEOF;
    }
    return c;
}

wchar_t *fgetws(wchar_t *s, int n, FILE *f) {
    int i = 0;
    while (i < n - 1) {
        wint_t c = fgetwc(f);
        if (c == WEOF)
            break;
        s[i++] = (wchar_t)c;
        if (c == L'\n')
            break;
    }
    if (!i)
        return NULL;
    s[i] = 0;
    return s;
}

int fputws(const wchar_t *s, FILE *f) {
    for (; *s; s++) {
        if (fputwc(*s, f) == WEOF)
            return -1;
    }
    return 0;
}

// streams dont keep an orientation, whatever is asked for is what they are
int fwide(FILE *f, int mode) {
    (void)f;
    return mode > 0 ? 1 : mode < 0 ? -1 : 0;
}
