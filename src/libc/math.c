#include "math.h"

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

double pow(double x, double y) {
    int n = (int)y;
    int negate_exp = 0;
    double result = 1.0;

    if ((double)n != y)
        return 1.0;

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
