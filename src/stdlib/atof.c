#include <libc/ctype.h>
#include <libc/stdlib.h>

double atof(const char *s) {
    double value = 0.0;
    double frac_scale = 1.0;
    int sign = 1;
    int exp_sign = 1;
    int exp = 0;

    if (!s) {
        return 0.0;
    }

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (isdigit((unsigned char)*s)) {
        value = value * 10.0 + (double)(*s - '0');
        s++;
    }

    if (*s == '.') {
        s++;
        while (isdigit((unsigned char)*s)) {
            value = value * 10.0 + (double)(*s - '0');
            frac_scale *= 10.0;
            s++;
        }
    }

    value /= frac_scale;

    if (*s == 'e' || *s == 'E') {
        s++;
        if (*s == '-') {
            exp_sign = -1;
            s++;
        } else if (*s == '+') {
            s++;
        }

        while (isdigit((unsigned char)*s)) {
            exp = exp * 10 + (*s - '0');
            s++;
        }
    }

    while (exp > 0) {
        if (exp_sign > 0) {
            value *= 10.0;
        } else {
            value /= 10.0;
        }
        exp--;
    }

    return (double)sign * value;
}
