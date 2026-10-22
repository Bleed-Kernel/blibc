#include <math.h>

double sqrt(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fsqrt  \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double sin(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fsin   \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double cos(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fcos   \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double tan(double x) {
    double s, c;
    __asm__ __volatile__ (
        "fldl %2 \n\t"
        "fsin    \n\t"
        "fstpl %0 \n\t"
        "fldl %2 \n\t"
        "fcos    \n\t"
        "fstpl %1"
        : "=m"(s), "=m"(c)
        : "m"(x)
    );
    return s / c;
}

double fabs(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fabs   \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double floor(double x) {
    long i = (long)x;

    if ((double)i > x)
        --i;

    return (double)i;
}

double ceil(double x) {
    long i = (long)x;

    if ((double)i < x)
        ++i;

    return (double)i;
}

double fmod(double x, double y) {
    long q;

    if (y == 0.0)
        return 0.0;

    q = (long)(x / y);
    return x - (double)q * y;
}

static double atan_approx(double x) {
    double ax = fabs(x);
    double r;

    if (ax > 1.0)
    {
        double inv = 1.0 / ax;
        r = 1.5707963267948966 - ((0.2447 + 0.0663 * inv) * inv);
    }
    else
    {
        r = ax * (0.7853981633974483 + 0.273 * (1.0 - ax));
    }

    return x < 0.0 ? -r : r;
}

double atan(double x) {
    return atan_approx(x);
}

double atan2(double y, double x) {
    if (x > 0.0)
        return atan_approx(y / x);

    if (x < 0.0)
    {
        if (y >= 0.0)
            return atan_approx(y / x) + 3.1415926535897932;

        return atan_approx(y / x) - 3.1415926535897932;
    }

    if (y > 0.0)
        return 1.5707963267948966;

    if (y < 0.0)
        return -1.5707963267948966;

    return 0.0;
}

double acos(double x) {
    if (x >= 1.0)
        return 0.0;

    if (x <= -1.0)
        return 3.1415926535897932;

    return atan2(sqrt(1.0 - x * x), x);
}

// y * log2(x), then 2 to the whole part and the fraction apart, f2xm1 only takes -1 to 1
static double pow_x87(double x, double y) {
    double result;
    __asm__ __volatile__ (
        "fldl %2            \n\t"
        "fldl %1            \n\t"
        "fyl2x              \n\t"
        "fld %%st(0)        \n\t"
        "frndint            \n\t"
        "fxch               \n\t"
        "fsub %%st(1), %%st \n\t"
        "f2xm1              \n\t"
        "fld1               \n\t"
        "faddp              \n\t"
        "fscale             \n\t"
        "fstp %%st(1)       \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x), "m"(y)
    );
    return result;
}

// e^x is 2^(x * log2 e), same split as pow
double exp(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1            \n\t"
        "fldl2e             \n\t"
        "fmulp              \n\t"
        "fld %%st(0)        \n\t"
        "frndint            \n\t"
        "fxch               \n\t"
        "fsub %%st(1), %%st \n\t"
        "f2xm1              \n\t"
        "fld1               \n\t"
        "faddp              \n\t"
        "fscale             \n\t"
        "fstp %%st(1)       \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double log(double x) {
    double result;
    __asm__ __volatile__ (
        "fldln2     \n\t"
        "fldl %1    \n\t"
        "fyl2x      \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double log10(double x) {
    double result;
    __asm__ __volatile__ (
        "fldlg2     \n\t"
        "fldl %1    \n\t"
        "fyl2x      \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double pow(double x, double y) {
    int n = (int)y;
    int negate_exp = 0;
    double result = 1.0;

    if ((double)n != y) {
        if (x > 0.0)
            return pow_x87(x, y);
        return 0.0;
    }

    if (n < 0)
    {
        negate_exp = 1;
        n = -n;
    }

    while (n > 0)
    {
        if (n & 1)
            result *= x;

        x *= x;
        n >>= 1;
    }

    return negate_exp ? 1.0 / result : result;
}
