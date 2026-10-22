#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

static int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_uint(const char *s, int base, int width, unsigned long *out, int *consumed) {
    unsigned long v = 0;
    int n = 0;
    int d;

    if (base == 16 && (width == 0 || width >= 2) &&
        s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        n = 2;
    }

    while ((width == 0 || n < width) && (d = digit_value(s[n])) >= 0 && d < base) {
        v = v * (unsigned long)base + (unsigned long)d;
        n++;
    }

    if (n == 0 || (n == 2 && base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))) return 0;

    *out = v;
    *consumed = n;
    return 1;
}

static int parse_int(const char *s, int base, int width, long *out, int *consumed) {
    int n = 0;
    int sign = 1;
    unsigned long u = 0;
    int uc = 0;

    if ((width == 0 || n < width) && (s[n] == '+' || s[n] == '-')) {
        if (s[n] == '-') sign = -1;
        n++;
    }

    if (!parse_uint(s + n, base, (width > 0) ? (width - n) : 0, &u, &uc)) {
        return 0;
    }

    n += uc;
    *consumed = n;
    *out = (long)u * (long)sign;
    return 1;
}

static int parse_float(const char *s, int width, double *out, int *consumed) {
    int n = 0;
    int sign = 1;
    double value = 0.0;
    double scale = 1.0;
    int has_digit = 0;
    int exp_sign = 1;
    int exp = 0;

    if ((width == 0 || n < width) && (s[n] == '+' || s[n] == '-')) {
        if (s[n] == '-') sign = -1;
        n++;
    }

    while ((width == 0 || n < width) && isdigit((unsigned char)s[n])) {
        value = value * 10.0 + (double)(s[n] - '0');
        n++;
        has_digit = 1;
    }

    if ((width == 0 || n < width) && s[n] == '.') {
        n++;
        while ((width == 0 || n < width) && isdigit((unsigned char)s[n])) {
            value = value * 10.0 + (double)(s[n] - '0');
            scale *= 10.0;
            n++;
            has_digit = 1;
        }
    }

    if (!has_digit) return 0;
    value /= scale;

    if ((width == 0 || n < width) && (s[n] == 'e' || s[n] == 'E')) {
        int en = n + 1;

        if ((width == 0 || en < width) && (s[en] == '+' || s[en] == '-')) {
            if (s[en] == '-') exp_sign = -1;
            en++;
        }

        if ((width == 0 || en < width) && isdigit((unsigned char)s[en])) {
            n = en;
            while ((width == 0 || n < width) && isdigit((unsigned char)s[n])) {
                exp = exp * 10 + (s[n] - '0');
                n++;
            }
        }
    }

    while (exp > 0) {
        if (exp_sign > 0) value *= 10.0;
        else value /= 10.0;
        exp--;
    }

    *out = value * (double)sign;
    *consumed = n;
    return 1;
}

// where the text comes from. a string is read in place
typedef struct {
    const char *s;
    FILE       *f;
    int         used;
    char        tok[256];
} src_t;

static int src_peek(src_t *in) {
    if (!in->f)
        return *in->s ? (unsigned char)*in->s : EOF;
    int c = fgetc(in->f);
    if (c != EOF)
        ungetc(c, in->f);
    return c;
}

static void src_take(src_t *in) {
    if (in->f) fgetc(in->f);
    else in->s++;
    in->used++;
}

static void src_skip_space(src_t *in) {
    int c;
    while ((c = src_peek(in)) != EOF && isspace(c))
        src_take(in);
}

static const char *src_field(src_t *in, int width) {
    if (!in->f)
        return in->s;

    int max = (int)sizeof(in->tok) - 1;
    if (width > 0 && width < max)
        max = width;

    int n = 0, c;
    while (n < max && (c = fgetc(in->f)) != EOF) {
        if (!isalnum(c) && c != '+' && c != '-' && c != '.') {
            ungetc(c, in->f);
            break;
        }
        in->tok[n++] = (char)c;
    }
    in->tok[n] = '\0';
    return in->tok;
}

static void src_used(src_t *in, int consumed) {
    if (!in->f)
        in->s += consumed;
    in->used += consumed;
}

static int scan(src_t *in, const char *fmt, va_list ap) {
    int assigned = 0;
    int matched_any = 0;
    const char *f = fmt;

    while (*f) {
        if (isspace((unsigned char)*f)) {
            while (isspace((unsigned char)*f)) f++;
            src_skip_space(in);
            continue;
        }

        if (*f != '%') {
            if (src_peek(in) != (unsigned char)*f) break;
            src_take(in);
            f++;
            continue;
        }

        f++;
        if (*f == '%') {
            src_skip_space(in);
            if (src_peek(in) != '%') break;
            src_take(in);
            f++;
            continue;
        }

        int suppress = 0;
        int width = 0;
        int long_mod = 0;
        int short_mod = 0;
        char conv;

        if (*f == '*') {
            suppress = 1;
            f++;
        }

        while (isdigit((unsigned char)*f)) {
            width = width * 10 + (*f - '0');
            f++;
        }

        while (*f == 'h' || *f == 'l' || *f == 'L' || *f == 'z' || *f == 'j' || *f == 't') {
            if (*f == 'h') short_mod++;
            else long_mod++;
            f++;
        }

        conv = *f++;
        if (conv == '\0') break;

        if (conv == 'n') {
            if (!suppress) *va_arg(ap, int *) = in->used;
            continue;
        }

        if (conv != 'c' && conv != '[') {
            src_skip_space(in);
        }
        if (src_peek(in) == EOF) {
            if (!matched_any) return -1;
            break;
        }

        if (conv == 'd' || conv == 'u' || conv == 'x' || conv == 'X' || conv == 'o' || conv == 'i' || conv == 'p') {
            int base = 10;
            int consumed = 0;
            const char *s = src_field(in, width);

            if (conv == 'x' || conv == 'X' || conv == 'p') base = 16;
            else if (conv == 'o') base = 8;

            if (conv == 'i') {
                int sign_chars = (s[0] == '+' || s[0] == '-') ? 1 : 0;
                if (s[sign_chars] == '0' && (s[sign_chars + 1] == 'x' || s[sign_chars + 1] == 'X')) {
                    base = 16;
                } else if (s[sign_chars] == '0') {
                    base = 8;
                }
            }

            if (conv == 'd' || conv == 'i') {
                long v;
                if (!parse_int(s, base, width, &v, &consumed)) break;
                if (!suppress) {
                    if (long_mod > 0) *va_arg(ap, long *) = v;
                    else if (short_mod == 1) *va_arg(ap, short *) = (short)v;
                    else if (short_mod > 1) *va_arg(ap, signed char *) = (signed char)v;
                    else *va_arg(ap, int *) = (int)v;
                    assigned++;
                }
            } else {
                unsigned long v;
                if (!parse_uint(s, base, width, &v, &consumed)) break;
                if (!suppress) {
                    if (conv == 'p') *va_arg(ap, void **) = (void *)v;
                    else if (long_mod > 0) *va_arg(ap, unsigned long *) = v;
                    else if (short_mod == 1) *va_arg(ap, unsigned short *) = (unsigned short)v;
                    else if (short_mod > 1) *va_arg(ap, unsigned char *) = (unsigned char)v;
                    else *va_arg(ap, unsigned int *) = (unsigned int)v;
                    assigned++;
                }
            }

            src_used(in, consumed);
            matched_any = 1;
            continue;
        }

        if (conv == 'f' || conv == 'e' || conv == 'E' || conv == 'g' || conv == 'G' || conv == 'a') {
            double dv;
            int consumed = 0;
            if (!parse_float(src_field(in, width), width, &dv, &consumed)) break;
            if (!suppress) {
                if (long_mod > 0) *va_arg(ap, double *) = dv;
                else *va_arg(ap, float *) = (float)dv;
                assigned++;
            }
            src_used(in, consumed);
            matched_any = 1;
            continue;
        }

        if (conv == 's' || conv == 'c' || conv == '[') {
            char set[256] = {0};
            int  invert = 0;
            if (conv == '[') {
                if (*f == '^') {
                    invert = 1;
                    f++;
                }
                if (*f == ']') {
                    set[']'] = 1;
                    f++;
                }
                while (*f && *f != ']') {
                    if (f[1] == '-' && f[2] && f[2] != ']') {
                        for (int ch = (unsigned char)f[0]; ch <= (unsigned char)f[2]; ch++)
                            set[ch] = 1;
                        f += 3;
                    } else {
                        set[(unsigned char)*f++] = 1;
                    }
                }
                if (*f == ']') f++;
            }

            int limit = width > 0 ? width : (conv == 'c' ? 1 : 0);
            char *out = suppress ? NULL : va_arg(ap, char *);
            int n = 0, c;

            while ((limit == 0 || n < limit) && (c = src_peek(in)) != EOF) {
                if (conv == 's' && isspace(c)) break;
                if (conv == '[' && set[c] == invert) break;
                if (out) out[n] = (char)c;
                src_take(in);
                n++;
            }

            if (n == 0 || (conv == 'c' && n < limit)) break;
            if (out) {
                if (conv != 'c') out[n] = '\0';
                assigned++;
            }
            matched_any = 1;
            continue;
        }

        break;
    }

    return assigned;
}

int vsscanf(const char *str, const char *fmt, va_list ap) {
    src_t in = { .s = str };
    return scan(&in, fmt, ap);
}

int sscanf(const char *str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsscanf(str, fmt, ap);
    va_end(ap);
    return r;
}

int vfscanf(FILE *stream, const char *fmt, va_list ap) {
    if (!stream)
        return -1;
    src_t in = { .f = stream };
    return scan(&in, fmt, ap);
}

int fscanf(FILE *stream, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfscanf(stream, fmt, ap);
    va_end(ap);
    return r;
}

int scanf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vfscanf(stdin, fmt, ap);
    va_end(ap);
    return r;
}
