#include <libc/stdlib.h>
#include <libc/ctype.h>
#include <libc/errno.h>
#include <libc/limits.h>

static int char_to_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return (c - 'a') + 10;
    if (c >= 'A' && c <= 'Z') return (c - 'A') + 10;
    return -1;
}

static int is_space(char c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

long strtol(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    int negative = 0;
    int any = 0;
    unsigned long acc = 0;
    unsigned long abs_long_min = ((unsigned long)LONG_MAX) + 1UL;
    unsigned long limit;
    unsigned long cutoff;
    unsigned long cutlim;

    while (isspace((unsigned char)*s)) s++;

    if (*s == '-' || *s == '+') {
        negative = (*s == '-');
        s++;
    }

    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    }

    if (base == 0) {
        base = (s[0] == '0') ? 8 : 10;
    }

    if (base < 2 || base > 36) {
        errno = EINVAL;
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }

    limit = negative ? abs_long_min : (unsigned long)LONG_MAX;
    cutoff = limit / (unsigned long)base;
    cutlim = limit % (unsigned long)base;

    for (; *s; s++) {
        int d = char_to_digit(*s);
        if (d < 0 || d >= base) break;

        if (any >= 0) {
            if (acc > cutoff || (acc == cutoff && (unsigned long)d > cutlim)) {
                any = -1;
                acc = limit;
                errno = ERANGE;
            } else {
                any = 1;
                acc = acc * (unsigned long)base + (unsigned long)d;
            }
        }
    }

    if (endptr) {
        *endptr = (char *)(any ? s : nptr);
    }

    if (any == -1) {
        return negative ? LONG_MIN : LONG_MAX;
    }

    if (negative) {
        if (acc == abs_long_min) return LONG_MIN;
        return -(long)acc;
    }

    return (long)acc;
}

unsigned long strtoul(const char* str, char** endptr, int base){
    const char* p = str;
    unsigned long result = 0;
    int negative = 0;
    int any_digits = 0;

    if (base < 0 || base == 1 || base > 36){
        if (endptr){
            *endptr = (char*)str;
        }
        return 0;
    }

    while (is_space(*p)){
        p++;
    }

    if (*p == '+'){
        p++;
    }else if (*p == '-'){
        negative = 1;
        p++;
    }

    if ((base == 0 || base == 16) && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')){
        p += 2;
        base = 16;
    }else if (base == 0){
        if (p[0] == '0'){
            base = 8;
        }else{
            base = 10;
        }
    }

    while (*p != '\0'){
        int digit = char_to_digit(*p);
        if (digit < 0 || digit >= base){
            break;
        }

        result = result * (unsigned long)base + (unsigned long)digit;
        any_digits = 1;
        p++;
    }

    if (!any_digits){
        if (endptr){
            *endptr = (char*)str;
        }
        return 0;
    }

    if (endptr){
        *endptr = (char*)p;
    }

    return negative ? (unsigned long)(-(long)result) : result;
}