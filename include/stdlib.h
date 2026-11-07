#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX     0x7fff

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

int atoi(const char *s);
double atof(const char *s);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
long atol(const char *s);
long long atoll(const char *s);
double strtod(const char *nptr, char **endptr);

// temp files, made with O_EXCL and random names
int   mkstemp(char *tmpl);
int   mkostemp(char *tmpl, int flags);
int   mkstemps(char *tmpl, int suffixlen);
int   mkostemps(char *tmpl, int suffixlen, int flags);
char *mkdtemp(char *tmpl);
float strtof(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);

// theres only the c locale, these are the plain ones
struct __locale_struct;
float       strtof_l(const char *nptr, char **endptr, struct __locale_struct *loc);
double      strtod_l(const char *nptr, char **endptr, struct __locale_struct *loc);
long double strtold_l(const char *nptr, char **endptr, struct __locale_struct *loc);
void abort(void) __attribute__((noreturn));

void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);

// multibyte is always utf-8
#define MB_CUR_MAX ((size_t)4)
int    mblen(const char *s, size_t n);
int    mbtowc(wchar_t *pwc, const char *s, size_t n);
int    wctomb(char *s, wchar_t wc);
size_t mbstowcs(wchar_t *dst, const char *src, size_t n);
size_t wcstombs(char *dst, const wchar_t *src, size_t n);
void* aligned_alloc(size_t align, size_t size);
int posix_memalign(void **out, size_t align, size_t size);

int abs(int x);
long labs(long x);
long long llabs(long long x);
div_t div(int num, int den);
ldiv_t ldiv(long num, long den);
lldiv_t lldiv(long long num, long long den);

int rand(void);
void srand(unsigned int seed);

long random(void);
void srandom(unsigned int seed);
char *initstate(unsigned int seed, char *state, size_t n);
char *setstate(char *state);
double drand48(void);
double erand48(unsigned short x[3]);
long lrand48(void);
long nrand48(unsigned short x[3]);
long mrand48(void);
long jrand48(unsigned short x[3]);
void srand48(long seed);

uint32_t arc4random(void);
void arc4random_buf(void *buf, size_t len);
uint32_t arc4random_uniform(uint32_t upper);

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
int system(const char *command);

char *getenv(const char *name);
char *realpath(const char *path, char *resolved);
int putenv(char *string);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

int atexit(void (*fn)(void));
void exit(int code) __attribute__((noreturn));
void _Exit(int code) __attribute__((noreturn));

#include <alloca.h>

#ifdef __cplusplus
}
#endif
