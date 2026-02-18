#include <libc/ctype.h>
#include <libc/stddef.h>
#include <libc/stdio.h>

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

int sscanf(const char *str, const char *fmt, ...) {
    va_list ap;
    int assigned = 0;
    const char *s = str;
    const char *f = fmt;

    va_start(ap, fmt);

    while (*f) {
        if (isspace((unsigned char)*f)) {
            while (isspace((unsigned char)*f)) f++;
            while (isspace((unsigned char)*s)) s++;
            continue;
        }

        if (*f != '%') {
            if (*s != *f) break;
            s++;
            f++;
            continue;
        }

        f++;
        if (*f == '%') {
            if (*s != '%') break;
            s++;
            f++;
            continue;
        }

        int suppress = 0;
        int width = 0;
        int long_mod = 0;
        char conv;

        if (*f == '*') {
            suppress = 1;
            f++;
        }

        while (isdigit((unsigned char)*f)) {
            width = width * 10 + (*f - '0');
            f++;
        }

        while (*f == 'h' || *f == 'l' || *f == 'L') {
            if (*f == 'l') long_mod++;
            f++;
        }

        conv = *f++;
        if (conv == '\0') break;

        if (conv != 'c' && conv != '[') {
            while (isspace((unsigned char)*s)) s++;
        }

        if (conv == 'd' || conv == 'u' || conv == 'x' || conv == 'X' || conv == 'o' || conv == 'i') {
            int base = 10;
            int consumed = 0;

            if (conv == 'x' || conv == 'X') base = 16;
            else if (conv == 'o') base = 8;

            if (conv == 'i') {
                int sign_chars = (s[0] == '+' || s[0] == '-') ? 1 : 0;
                if (s[sign_chars] == '0' && (s[sign_chars + 1] == 'x' || s[sign_chars + 1] == 'X')) {
                    base = 16;
                } else if (s[sign_chars] == '0') {
                    base = 8;
                }
            }

            if (conv == 'd') {
                long v;
                if (!parse_int(s, 10, width, &v, &consumed)) break;
                if (!suppress) {
                    if (long_mod > 0) *va_arg(ap, long *) = v;
                    else *va_arg(ap, int *) = (int)v;
                    assigned++;
                }
            } else if (conv == 'u' || conv == 'x' || conv == 'X' || conv == 'o') {
                unsigned long v;
                if (!parse_uint(s, base, width, &v, &consumed)) break;
                if (!suppress) {
                    if (long_mod > 0) *va_arg(ap, unsigned long *) = v;
                    else *va_arg(ap, unsigned int *) = (unsigned int)v;
                    assigned++;
                }
            } else {
                long v;
                if (!parse_int(s, base, width, &v, &consumed)) break;
                if (!suppress) {
                    if (long_mod > 0) *va_arg(ap, long *) = v;
                    else *va_arg(ap, int *) = (int)v;
                    assigned++;
                }
            }

            s += consumed;
            continue;
        }

        if (conv == 'f' || conv == 'e' || conv == 'E' || conv == 'g' || conv == 'G') {
            double dv;
            int consumed = 0;
            if (!parse_float(s, width, &dv, &consumed)) break;
            if (!suppress) {
                if (long_mod > 0) *va_arg(ap, double *) = dv;
                else *va_arg(ap, float *) = (float)dv;
                assigned++;
            }
            s += consumed;
            continue;
        }

        if (conv == 's') {
            int n = 0;
            char *out = suppress ? NULL : va_arg(ap, char *);

            while (*s && !isspace((unsigned char)*s) && (width == 0 || n < width)) {
                if (!suppress) out[n] = *s;
                s++;
                n++;
            }
            if (n == 0) break;
            if (!suppress) {
                out[n] = '\0';
                assigned++;
            }
            continue;
        }

        if (conv == 'c') {
            int n = (width > 0) ? width : 1;
            char *out = suppress ? NULL : va_arg(ap, char *);
            for (int i = 0; i < n; i++) {
                if (*s == '\0') goto done;
                if (!suppress) out[i] = *s;
                s++;
            }
            if (!suppress) assigned++;
            continue;
        }

        break;
    }

done:
    va_end(ap);
    if (assigned == 0 && *s == '\0') return -1;
    return assigned;
}
