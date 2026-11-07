#include <math.h>
#include <stdint.h>
#include <string.h>

int signgam;


static double poly_lanczos(double x) {
    static const double g = 7.0;
    static const double c[9] = {
        0.99999999999980993, 676.5203681218851, -1259.1392167224028, 771.32342877765313,
        -176.61502916214059, 12.507343278686905, -0.13857109526572012, 9.9843695780195716e-6,
        1.5056327351493116e-7,
    };
    (void)g;
    double a = c[0];
    for (int i = 1; i < 9; i++)
        a += c[i] / (x + i);
    return a;
}

double tgamma(double x) {
    if (x != x) return x;
    if (x == floor(x) && x <= 0) return NAN;
    if (x < 0.5)
        return M_PI / (sin(M_PI * x) * tgamma(1 - x));
    if (x > 171.7) return INFINITY;
    x -= 1;
    double t = x + 7.5;
    return sqrt(2 * M_PI) * pow(t, x + 0.5) * exp(-t) * poly_lanczos(x);
}

double lgamma_r(double x, int *sign) {
    *sign = 1;
    if (x != x) return x;
    if (x == floor(x) && x <= 0) return INFINITY;
    if (x < 0.5) {
        double s = sin(M_PI * x);
        if (s < 0) { *sign = -1; s = -s; }
        int dummy;
        return log(M_PI / s) - lgamma_r(1 - x, &dummy);
    }
    x -= 1;
    double t = x + 7.5;
    return 0.5 * log(2 * M_PI) + (x + 0.5) * log(t) - t + log(poly_lanczos(x));
}

double lgamma(double x) { return lgamma_r(x, &signgam); }

double expm1(double x) {
    if (fabs(x) < 1e-5) return x + x * x / 2 + x * x * x / 6;
    return exp(x) - 1;
}

// kahans trick keeps log1p right for small x
double log1p(double x) {
    double u = 1 + x;
    if (u == 1) return x;
    return log(u) * (x / (u - 1));
}

double asinh(double x) {
    double a = fabs(x), r;
    if (a > 1e8) r = log(a) + M_LN2;
    else r = log1p(a + a * a / (1 + sqrt(1 + a * a)));
    return x < 0 ? -r : r;
}

double acosh(double x) {
    if (x < 1) return NAN;
    if (x > 1e8) return log(x) + M_LN2;
    return log(x + sqrt(x * x - 1));
}

double atanh(double x) {
    double a = fabs(x);
    if (a > 1) return NAN;
    double r = 0.5 * log1p(2 * a / (1 - a));
    return x < 0 ? -r : r;
}

// series for small x, continued fraction for the tail
double erf(double x) {
    if (x != x) return x;
    double a = fabs(x);
    if (a < 2.0) {
        double sum = 0, term = a, x2 = a * a;
        for (int n = 0; n < 60; n++) {
            double add = term / (2 * n + 1);
            sum += add;
            if (fabs(add) < 1e-17 * fabs(sum)) break;
            term *= -x2 / (n + 1);
        }
        double r = sum * M_2_SQRTPI;
        return x < 0 ? -r : r;
    }
    double r = 1 - erfc(a);
    return x < 0 ? -r : r;
}

double erfc(double x) {
    if (x != x) return x;
    if (x < 2.0) return 1 - erf(x);
    if (x > 27) return 0;
    // lentz on erfc(x) = exp(-x^2)/sqrt(pi) * 1/(x + 1/2/(x + 1/(x + 3/2/(x + ...
    double tiny = 1e-300, f = x, c = x, dd = 0;
    for (int n = 1; n < 300; n++) {
        double an = n * 0.5;
        dd = x + an * dd; if (dd == 0) dd = tiny; dd = 1 / dd;
        c = x + an / c;   if (c == 0) c = tiny;
        double delta = c * dd;
        f *= delta;
        if (fabs(delta - 1) < 1e-16) break;
    }
    return exp(-x * x) / (f * 1.7724538509055160273);
}

double remainder(double x, double y) {
    if (y == 0 || isinf(x) || x != x || y != y) return NAN;
    double n = rint(x / y);
    return x - n * y;
}

double remquo(double x, double y, int *q) {
    double n = rint(x / y);
    *q = (int)((long long)n & 0x7fffffff) * (n < 0 ? -1 : 1);
    return x - n * y;
}

double fdim(double x, double y) {
    if (x != x || y != y) return NAN;
    return x > y ? x - y : 0;
}

double fma(double x, double y, double z) {
    return (double)((long double)x * y + z);
}

double nextafter(double x, double y) {
    if (x != x || y != y) return x + y;
    if (x == y) return y;
    union { double d; uint64_t u; } v = { x };
    if (x == 0) {
        v.u = 1;
        return y > 0 ? v.d : -v.d;
    }
    if ((x < y) == (x > 0)) v.u++;
    else v.u--;
    return v.d;
}

double nexttoward(double x, long double y) { return nextafter(x, (double)y); }

int ilogb(double x) {
    if (x == 0) return FP_ILOGB0;
    if (x != x) return FP_ILOGBNAN;
    if (isinf(x)) return 2147483647;
    int e;
    frexp(x, &e);
    return e - 1;
}

double logb(double x) {
    if (x == 0) return -INFINITY;
    if (isinf(x)) return INFINITY;
    if (x != x) return x;
    return (double)ilogb(x);
}

double significand(double x) { return scalbn(x, -ilogb(x)); }

double nan(const char *tag) { (void)tag; return NAN; }

double scalbln(double x, long n) {
    if (n > 100000) n = 100000;
    if (n < -100000) n = -100000;
    return scalbn(x, (int)n);
}

long long llround(double x) { return (long long)round(x); }

void sincos(double x, double *s, double *c) { *s = sin(x); *c = cos(x); }

double exp10(double x) { return pow(10.0, x); }

intmax_t imaxabs(intmax_t n) { return n < 0 ? -n : n; }

#include <inttypes.h>
imaxdiv_t imaxdiv(intmax_t n, intmax_t d) { imaxdiv_t r = { n / d, n % d }; return r; }

long double sqrtl(long double x) { return (long double)sqrt((double)x); }
float cbrtf(float x) { return (float)cbrt(x); }
long double cbrtl(long double x) { return (long double)cbrt((double)x); }
long double sinl(long double x) { return (long double)sin((double)x); }
long double cosl(long double x) { return (long double)cos((double)x); }
long double tanl(long double x) { return (long double)tan((double)x); }
long double asinl(long double x) { return (long double)asin((double)x); }
long double acosl(long double x) { return (long double)acos((double)x); }
long double atanl(long double x) { return (long double)atan((double)x); }
float sinhf(float x) { return (float)sinh(x); }
long double sinhl(long double x) { return (long double)sinh((double)x); }
float coshf(float x) { return (float)cosh(x); }
long double coshl(long double x) { return (long double)cosh((double)x); }
float tanhf(float x) { return (float)tanh(x); }
long double tanhl(long double x) { return (long double)tanh((double)x); }
float asinhf(float x) { return (float)asinh(x); }
long double asinhl(long double x) { return (long double)asinh((double)x); }
float acoshf(float x) { return (float)acosh(x); }
long double acoshl(long double x) { return (long double)acosh((double)x); }
float atanhf(float x) { return (float)atanh(x); }
long double atanhl(long double x) { return (long double)atanh((double)x); }
long double expl(long double x) { return (long double)exp((double)x); }
float exp2f(float x) { return (float)exp2(x); }
long double exp2l(long double x) { return (long double)exp2((double)x); }
float expm1f(float x) { return (float)expm1(x); }
long double expm1l(long double x) { return (long double)expm1((double)x); }
long double logl(long double x) { return (long double)log((double)x); }
long double log2l(long double x) { return (long double)log2((double)x); }
long double log10l(long double x) { return (long double)log10((double)x); }
float log1pf(float x) { return (float)log1p(x); }
long double log1pl(long double x) { return (long double)log1p((double)x); }
float logbf(float x) { return (float)logb(x); }
long double logbl(long double x) { return (long double)logb((double)x); }
long double fabsl(long double x) { return (long double)fabs((double)x); }
long double floorl(long double x) { return (long double)floor((double)x); }
long double ceill(long double x) { return (long double)ceil((double)x); }
long double truncl(long double x) { return (long double)trunc((double)x); }
long double roundl(long double x) { return (long double)round((double)x); }
long double rintl(long double x) { return (long double)rint((double)x); }
long double nearbyintl(long double x) { return (long double)nearbyint((double)x); }
float erff(float x) { return (float)erf(x); }
long double erfl(long double x) { return (long double)erf((double)x); }
float erfcf(float x) { return (float)erfc(x); }
long double erfcl(long double x) { return (long double)erfc((double)x); }
float lgammaf(float x) { return (float)lgamma(x); }
long double lgammal(long double x) { return (long double)lgamma((double)x); }
float tgammaf(float x) { return (float)tgamma(x); }
long double tgammal(long double x) { return (long double)tgamma((double)x); }
float hypotf(float x, float y) { return (float)hypot(x, y); }
long double hypotl(long double x, long double y) { return (long double)hypot((double)x, (double)y); }
long double atan2l(long double x, long double y) { return (long double)atan2((double)x, (double)y); }
long double fmodl(long double x, long double y) { return (long double)fmod((double)x, (double)y); }
long double fminl(long double x, long double y) { return (long double)fmin((double)x, (double)y); }
long double fmaxl(long double x, long double y) { return (long double)fmax((double)x, (double)y); }
float copysignf(float x, float y) { return (float)copysign(x, y); }
long double copysignl(long double x, long double y) { return (long double)copysign((double)x, (double)y); }
long double powl(long double x, long double y) { return (long double)pow((double)x, (double)y); }
float remainderf(float x, float y) { return (float)remainder(x, y); }
long double remainderl(long double x, long double y) { return (long double)remainder((double)x, (double)y); }
float fdimf(float x, float y) { return (float)fdim(x, y); }
long double fdiml(long double x, long double y) { return (long double)fdim((double)x, (double)y); }
float nextafterf(float x, float y) { return (float)nextafter(x, y); }
long double nextafterl(long double x, long double y) { return (long double)nextafter((double)x, (double)y); }
float fmaf(float x, float y, float z) { return (float)((double)x * y + z); }
long double fmal(long double x, long double y, long double z) { return x * y + z; }
float remquof(float x, float y, int *q) { return (float)remquo(x, y, q); }
long double remquol(long double x, long double y, int *q) { return remquo((double)x, (double)y, q); }
int ilogbf(float x) { return ilogb(x); }
int ilogbl(long double x) { return ilogb((double)x); }
float nexttowardf(float x, long double y) { return nextafterf(x, (float)y); }
long double nexttowardl(long double x, long double y) { return nextafter((double)x, (double)y); }
float nanf(const char *tag) { (void)tag; return __builtin_nanf(""); }
long double nanl(const char *tag) { (void)tag; return __builtin_nanl(""); }
float scalblnf(float x, long n) { return (float)scalbln(x, n); }
long double scalblnl(long double x, long n) { return scalbln((double)x, n); }
float scalbnf(float x, int n) { return (float)scalbn(x, n); }
long double scalbnl(long double x, int n) { return ldexpl(x, n); }
float frexpf(float x, int *e) { return (float)frexp(x, e); }
float modff(float x, float *ip) { double i; float r = (float)modf(x, &i); *ip = (float)i; return r; }
long double modfl(long double x, long double *ip) { double i; long double r = modf((double)x, &i); *ip = i; return r; }
long long llroundf(float x) { return llround(x); }
long long llroundl(long double x) { return llround((double)x); }
long long llrintf(float x) { return (long long)rintf(x); }
long long llrintl(long double x) { return (long long)rint((double)x); }
long lrintl(long double x) { return (long)rint((double)x); }
long lroundl(long double x) { return (long)round((double)x); }
void sincosf(float x, float *s, float *c) { *s = sinf(x); *c = cosf(x); }
float exp10f(float x) { return (float)exp10(x); }
float lgammaf_r(float x, int *sign) { return (float)lgamma_r(x, sign); }
