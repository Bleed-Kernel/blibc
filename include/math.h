#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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
double rint(double x);
double nearbyint(double x);
long lrint(double x);
long long llrint(double x);
long lround(double x);
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
float rintf(float x);
float nearbyintf(float x);
long lrintf(float x);
long lroundf(float x);
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

// the rest of c99, float and long double ones go through double
typedef float  float_t;
typedef double double_t;

#define FP_ILOGB0       (-2147483647 - 1)
#define FP_ILOGBNAN     (-2147483647 - 1)
#define MATH_ERRNO      1
#define MATH_ERREXCEPT  2
#define math_errhandling 2
#define HUGE_VALL       __builtin_huge_vall()
#define M_2_SQRTPI      1.12837916709551257390
#define isgreater(x, y)      __builtin_isgreater(x, y)
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
#define isless(x, y)         __builtin_isless(x, y)
#define islessequal(x, y)    __builtin_islessequal(x, y)
#define islessgreater(x, y)  __builtin_islessgreater(x, y)
#define isunordered(x, y)    __builtin_isunordered(x, y)

extern int signgam;

long double sqrtl(long double x);
float cbrtf(float x);
long double cbrtl(long double x);
long double sinl(long double x);
long double cosl(long double x);
long double tanl(long double x);
long double asinl(long double x);
long double acosl(long double x);
long double atanl(long double x);
float sinhf(float x);
long double sinhl(long double x);
float coshf(float x);
long double coshl(long double x);
float tanhf(float x);
long double tanhl(long double x);
double asinh(double x);
float asinhf(float x);
long double asinhl(long double x);
double acosh(double x);
float acoshf(float x);
long double acoshl(long double x);
double atanh(double x);
float atanhf(float x);
long double atanhl(long double x);
long double expl(long double x);
float exp2f(float x);
long double exp2l(long double x);
double expm1(double x);
float expm1f(float x);
long double expm1l(long double x);
long double logl(long double x);
long double log2l(long double x);
long double log10l(long double x);
double log1p(double x);
float log1pf(float x);
long double log1pl(long double x);
double logb(double x);
float logbf(float x);
long double logbl(long double x);
long double fabsl(long double x);
long double floorl(long double x);
long double ceill(long double x);
long double truncl(long double x);
long double roundl(long double x);
long double rintl(long double x);
long double nearbyintl(long double x);
double erf(double x);
float erff(float x);
long double erfl(long double x);
double erfc(double x);
float erfcf(float x);
long double erfcl(long double x);
double lgamma(double x);
float lgammaf(float x);
long double lgammal(long double x);
double tgamma(double x);
float tgammaf(float x);
long double tgammal(long double x);
float hypotf(float x, float y);
long double hypotl(long double x, long double y);
long double atan2l(long double x, long double y);
long double fmodl(long double x, long double y);
long double fminl(long double x, long double y);
long double fmaxl(long double x, long double y);
float copysignf(float x, float y);
long double copysignl(long double x, long double y);
long double powl(long double x, long double y);
double remainder(double x, double y);
float remainderf(float x, float y);
long double remainderl(long double x, long double y);
double fdim(double x, double y);
float fdimf(float x, float y);
long double fdiml(long double x, long double y);
double nextafter(double x, double y);
float nextafterf(float x, float y);
long double nextafterl(long double x, long double y);
double fma(double x, double y, double z);
float fmaf(float x, float y, float z);
long double fmal(long double x, long double y, long double z);
double remquo(double x, double y, int *q);
float remquof(float x, float y, int *q);
long double remquol(long double x, long double y, int *q);
int ilogb(double x);
int ilogbf(float x);
int ilogbl(long double x);
double nexttoward(double x, long double y);
float nexttowardf(float x, long double y);
long double nexttowardl(long double x, long double y);
double nan(const char *tag);
float nanf(const char *tag);
long double nanl(const char *tag);
double scalbln(double x, long n);
float scalblnf(float x, long n);
long double scalblnl(long double x, long n);
float scalbnf(float x, int n);
long double scalbnl(long double x, int n);
float frexpf(float x, int *e);
float modff(float x, float *ip);
long double modfl(long double x, long double *ip);
long long llround(double x);
long long llroundf(float x);
long long llroundl(long double x);
long long llrintf(float x);
long long llrintl(long double x);
long lrintl(long double x);
long lroundl(long double x);
void sincos(double x, double *s, double *c);
void sincosf(float x, float *s, float *c);
double exp10(double x);
float exp10f(float x);
double significand(double x);
double lgamma_r(double x, int *sign);
float lgammaf_r(float x, int *sign);

#ifdef __cplusplus
}
#endif
