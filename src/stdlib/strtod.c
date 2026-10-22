#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

static const double tens[] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256 };

static double pow10_of(int e) {
    double r = 1.0;
    for (int i = 0; e && i < 9; i++, e >>= 1) {
        if (e & 1)
            r *= tens[i];
    }
    return r;
}

static int match(const char *s, const char *word) {
    int n = 0;
    while (word[n] && tolower((unsigned char)s[n]) == word[n])
        n++;
    return word[n] ? 0 : n;
}

// 0x1.8p3 style
static double hex_part(const char *s, const char **end) {
    uint64_t m = 0;
    int      e = 0, any = 0;

    for (;; s++) {
        int d = isdigit((unsigned char)*s) ? *s - '0'
              : isxdigit((unsigned char)*s) ? tolower((unsigned char)*s) - 'a' + 10 : -1;
        if (d < 0)
            break;
        if (m >> 60) e += 4;
        else m = m * 16 + (uint64_t)d;
        any = 1;
    }
    if (*s == '.') {
        for (s++;; s++) {
            int d = isdigit((unsigned char)*s) ? *s - '0'
                  : isxdigit((unsigned char)*s) ? tolower((unsigned char)*s) - 'a' + 10 : -1;
            if (d < 0)
                break;
            if (!(m >> 60)) {
                m = m * 16 + (uint64_t)d;
                e -= 4;
            }
            any = 1;
        }
    }
    if (!any)
        return -1.0;

    if (*s == 'p' || *s == 'P') {
        const char *p = s + 1;
        int sign = 1, x = 0;
        if (*p == '+' || *p == '-')
            sign = *p++ == '-' ? -1 : 1;
        if (isdigit((unsigned char)*p)) {
            while (isdigit((unsigned char)*p)) {
                if (x < 100000) x = x * 10 + (*p - '0');
                p++;
            }
            e += sign * x;
            s = p;
        }
    }
    *end = s;

    double v = (double)m;
    for (; e > 0; e--) v *= 2.0;
    for (; e < 0; e++) v *= 0.5;
    return v;
}

double strtod(const char *nptr, char **endptr) {
    const char *s = nptr;
    int neg = 0;
    double v;

    while (isspace((unsigned char)*s)) s++;
    if (*s == '+' || *s == '-')
        neg = *s++ == '-';

    int n;
    if ((n = match(s, "infinity")) || (n = match(s, "inf"))) {
        v = __builtin_inf();
        s += n;
        goto done;
    }
    if ((n = match(s, "nan"))) {
        v = __builtin_nan("");
        s += n;
        goto done;
    }

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        const char *end;
        v = hex_part(s + 2, &end);
        if (v < 0) {
            v = 0.0;
            s++;
        } else {
            s = end;
        }
        goto done;
    }

    uint64_t m = 0;
    int digits = 0, e = 0, any = 0;
    for (; isdigit((unsigned char)*s); s++, any = 1) {
        if (digits < 19) {
            m = m * 10 + (uint64_t)(*s - '0');
            if (m) digits++;
        } else {
            e++;
        }
    }
    if (*s == '.') {
        for (s++; isdigit((unsigned char)*s); s++, any = 1) {
            if (digits < 19) {
                m = m * 10 + (uint64_t)(*s - '0');
                if (m) digits++;
                e--;
            }
        }
    }
    if (!any) {
        if (endptr) *endptr = (char *)nptr;
        return 0.0;
    }

    if (*s == 'e' || *s == 'E') {
        const char *p = s + 1;
        int sign = 1, x = 0;
        if (*p == '+' || *p == '-')
            sign = *p++ == '-' ? -1 : 1;
        if (isdigit((unsigned char)*p)) {
            while (isdigit((unsigned char)*p)) {
                if (x < 100000) x = x * 10 + (*p - '0');
                p++;
            }
            e += sign * x;
            s = p;
        }
    }

    v = (double)m;
    if (m == 0) {
        // nothing to scale
    } else if (e > 0) {
        if (e > 400) v = __builtin_inf();
        else v *= pow10_of(e);
    } else if (e < 0) {
        // past 1e-308 the divisor itself would blow up, so its done in two goes
        int down = -e;
        if (down > 400) {
            v = 0.0;
        } else {
            if (down > 300) {
                v /= 1e300;
                down -= 300;
            }
            v /= pow10_of(down);
        }
    }

    if (m && (v == 0.0 || v == __builtin_inf()))
        errno = ERANGE;

done:
    if (endptr) *endptr = (char *)s;
    return neg ? -v : v;
}

float strtof(const char *nptr, char **endptr) {
    return (float)strtod(nptr, endptr);
}

long double strtold(const char *nptr, char **endptr) {
    return strtod(nptr, endptr);
}
