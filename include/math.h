#pragma once

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

// the same answers worked out from the bits, i do want to be able to make bleed
// compile with tcc but for now this is fine i just did it while i remembered to
int __fpclassify(double x);
int __signbit(double x);

#ifdef __TINYC__
#define HUGE_VAL    (1e500)
#define HUGE_VALF   (1e50f)
#define INFINITY    (1e50f)
#define NAN         (0.0f / 0.0f)

#define isnan(x)        ((x) != (x))
#define isinf(x)        (__fpclassify(x) == FP_INFINITE)
#define isfinite(x)     (__fpclassify(x) >= FP_ZERO)
#define isnormal(x)     (__fpclassify(x) == FP_NORMAL)
#define signbit(x)      __signbit(x)
#define fpclassify(x)   __fpclassify(x)
#else
#define HUGE_VAL    __builtin_huge_val()
#define HUGE_VALF   __builtin_huge_valf()
#define INFINITY    __builtin_inff()
#define NAN         __builtin_nanf("")

#define isnan(x)        __builtin_isnan(x)
#define isinf(x)        __builtin_isinf_sign(x)
#define isfinite(x)     __builtin_isfinite(x)
#define isnormal(x)     __builtin_isnormal(x)
#define signbit(x)      __builtin_signbit(x)
#define fpclassify(x)   __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, x)
#endif

#define M_E         2.7182818284590452354
#define M_LOG2E     1.4426950408889634074
#define M_LOG10E    0.43429448190325182765
#define M_LN2       0.69314718055994530942
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.78539816339744830962
#define M_1_PI      0.31830988618379067154
#define M_2_PI      0.63661977236758134308
#define M_SQRT2     1.41421356237309504880
#define M_SQRT1_2   0.70710678118654752440

double sqrt(double x);
double cbrt(double x);
double hypot(double x, double y);
double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y, double x);
double sinh(double x);
double cosh(double x);
double tanh(double x);
double fabs(double x);
double floor(double x);
double ceil(double x);
double trunc(double x);
double round(double x);
double fmod(double x, double y);
double fmin(double x, double y);
double fmax(double x, double y);
double copysign(double x, double y);
double pow(double x, double y);
double exp(double x);
double exp2(double x);
double log(double x);
double log2(double x);
double log10(double x);
double frexp(double x, int *exp);
double ldexp(double x, int exp);
double scalbn(double x, int n);
double modf(double x, double *ip);

long double ldexpl(long double x, int exp);
long double frexpl(long double x, int *exp);

float sqrtf(float x);
float sinf(float x);
float cosf(float x);
float tanf(float x);
float fabsf(float x);
float floorf(float x);
float ceilf(float x);
float roundf(float x);
float truncf(float x);
float fmodf(float x, float y);
float atanf(float x);
float atan2f(float y, float x);
float asinf(float x);
float acosf(float x);
float powf(float x, float y);
float expf(float x);
float logf(float x);
float log2f(float x);
float log10f(float x);
float fminf(float x, float y);
float fmaxf(float x, float y);
float ldexpf(float x, int e);
