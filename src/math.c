#include <math.h>
#include <stdint.h>

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
    if (!(fabs(x) < 4503599627370496.0))
        return x;
    double i = (double)(long)x;
    return i > x ? i - 1.0 : i;
}

double ceil(double x) {
    if (!(fabs(x) < 4503599627370496.0))
        return x;
    double i = (double)(long)x;
    return i < x ? i + 1.0 : i;
}

double trunc(double x) {
    if (!(fabs(x) < 4503599627370496.0))
        return x;
    return (double)(long)x;
}

double round(double x) {
    return x < 0.0 ? -floor(-x + 0.5) : floor(x + 0.5);
}

// fprem only takes off so much per go, it says so in c2 and gets run again
double fmod(double x, double y) {
    double result;
    __asm__ __volatile__ (
        "fldl %2            \n\t"
        "fldl %1            \n\t"
        "1:                 \n\t"
        "fprem              \n\t"
        "fnstsw %%ax        \n\t"
        "testw $0x400, %%ax \n\t"
        "jnz 1b             \n\t"
        "fstp %%st(1)       \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x), "m"(y)
        : "ax"
    );
    return result;
}

double atan2(double y, double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1    \n\t"
        "fldl %2    \n\t"
        "fpatan     \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(y), "m"(x)
    );
    return result;
}

double atan(double x) {
    return atan2(x, 1.0);
}

double asin(double x) {
    if (x > 1.0 || x < -1.0)
        return NAN;
    return atan2(x, sqrt(1.0 - x * x));
}

double acos(double x) {
    if (x > 1.0 || x < -1.0)
        return NAN;
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

double log2(double x) {
    double result;
    __asm__ __volatile__ (
        "fld1       \n\t"
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
    if (y == 0.0)
        return 1.0;
    if (isnan(x) || isnan(y))
        return NAN;
    if (fabs(y) >= 2147483648.0) {
        int odd = x < 0.0 && fmod(y, 2.0) != 0.0;
        if (x < 0.0 && floor(y) != y)
            return NAN;
        double r = x == 0.0 ? (y > 0 ? 0.0 : INFINITY) : pow_x87(fabs(x), y);
        return odd ? -r : r;
    }

    int n = (int)y;
    int negate_exp = 0;
    double result = 1.0;

    if ((double)n != y) {
        if (x > 0.0)
            return pow_x87(x, y);
        if (x == 0.0)
            return y > 0.0 ? 0.0 : INFINITY;
        return NAN;
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

double exp2(double x) {
    return pow(2.0, x);
}

double sinh(double x) {
    double e = exp(x);
    return (e - 1.0 / e) * 0.5;
}

double cosh(double x) {
    double e = exp(x);
    return (e + 1.0 / e) * 0.5;
}

double tanh(double x) {
    if (x > 20.0)  return 1.0;
    if (x < -20.0) return -1.0;
    double e = exp(2.0 * x);
    return (e - 1.0) / (e + 1.0);
}

double hypot(double x, double y) {
    x = fabs(x);
    y = fabs(y);
    if (isinf(x) || isinf(y))
        return INFINITY;
    double big = x > y ? x : y, small = x > y ? y : x;
    if (big == 0.0)
        return 0.0;
    small /= big;
    return big * sqrt(1.0 + small * small);
}

double cbrt(double x) {
    if (x == 0.0 || !isfinite(x))
        return x;
    double r = pow_x87(fabs(x), 1.0 / 3.0);
    r -= (r * r * r - fabs(x)) / (3.0 * r * r);
    return x < 0.0 ? -r : r;
}

double fmin(double x, double y) {
    if (isnan(x)) return y;
    if (isnan(y)) return x;
    return x < y ? x : y;
}

double fmax(double x, double y) {
    if (isnan(x)) return y;
    if (isnan(y)) return x;
    return x > y ? x : y;
}

double copysign(double x, double y) {
    return __builtin_copysign(x, y);
}

double frexp(double x, int *exp) {
    union { double d; uint64_t u; } v = { x };
    int e = (int)((v.u >> 52) & 0x7ff);

    if (e == 0) {
        if (x == 0.0) {
            *exp = 0;
            return x;
        }
        x = frexp(x * 18446744073709551616.0, exp);  // 2^64
        *exp -= 64;
        return x;
    }
    if (e == 0x7ff) {
        *exp = 0;
        return x;
    }

    *exp = e - 1022;
    v.u = (v.u & ~(0x7ffULL << 52)) | (1022ULL << 52);
    return v.d;
}

double ldexp(double x, int exp) {
    if (x == 0.0 || !isfinite(x))
        return x;
    // in steps the multiplier can always hold
    while (exp > 1000)  { x *= 0x1p1000;  exp -= 1000; }
    while (exp < -1000) { x *= 0x1p-1000; exp += 1000; }
    union { double d; uint64_t u; } scale = { 0 };
    if (exp >= -1022) {
        scale.u = (uint64_t)(exp + 1023) << 52;
        return x * scale.d;
    }
    return x * 0x1p-1000 * ldexp(1.0, exp + 1000);
}

double scalbn(double x, int n) {
    return ldexp(x, n);
}

double modf(double x, double *ip) {
    double i = trunc(x);
    if (ip) *ip = i;
    return isinf(x) ? copysign(0.0, x) : x - i;
}

float sqrtf(float x)            { return (float)sqrt(x); }
float sinf(float x)             { return (float)sin(x); }
float cosf(float x)             { return (float)cos(x); }
float tanf(float x)             { return (float)tan(x); }
float fabsf(float x)            { return x < 0.0f ? -x : x; }
float floorf(float x)           { return (float)floor(x); }
float ceilf(float x)            { return (float)ceil(x); }
float roundf(float x)           { return (float)round(x); }
float truncf(float x)           { return (float)trunc(x); }
float fmodf(float x, float y)   { return (float)fmod(x, y); }
float atanf(float x)            { return (float)atan(x); }
float atan2f(float y, float x)  { return (float)atan2(y, x); }
float asinf(float x)            { return (float)asin(x); }
float acosf(float x)            { return (float)acos(x); }
float powf(float x, float y)    { return (float)pow(x, y); }
float expf(float x)             { return (float)exp(x); }
float logf(float x)             { return (float)log(x); }
float log2f(float x)            { return (float)log2(x); }
float log10f(float x)           { return (float)log10(x); }
float fminf(float x, float y)   { return (float)fmin(x, y); }
float fmaxf(float x, float y)   { return (float)fmax(x, y); }
float ldexpf(float x, int e)    { return (float)ldexp(x, e); }
