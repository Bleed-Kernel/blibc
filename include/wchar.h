#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif
typedef __WINT_TYPE__ wint_t;

typedef struct {
    unsigned state;
} mbstate_t;

#define WEOF        ((wint_t)-1)
#ifndef WCHAR_MIN
#define WCHAR_MIN   __WCHAR_MIN__
#define WCHAR_MAX   __WCHAR_MAX__
#endif

struct tm;

size_t   wcslen(const wchar_t *s);
size_t   wcsnlen(const wchar_t *s, size_t n);
wchar_t *wcscpy(wchar_t *dst, const wchar_t *src);
wchar_t *wcsncpy(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wcscat(wchar_t *dst, const wchar_t *src);
wchar_t *wcsncat(wchar_t *dst, const wchar_t *src, size_t n);
int      wcscmp(const wchar_t *a, const wchar_t *b);
int      wcsncmp(const wchar_t *a, const wchar_t *b, size_t n);
int      wcscoll(const wchar_t *a, const wchar_t *b);
size_t   wcsxfrm(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wcschr(const wchar_t *s, wchar_t c);
wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
wchar_t *wcsstr(const wchar_t *hay, const wchar_t *needle);
size_t   wcsspn(const wchar_t *s, const wchar_t *accept);
size_t   wcscspn(const wchar_t *s, const wchar_t *reject);
wchar_t *wcspbrk(const wchar_t *s, const wchar_t *accept);
wchar_t *wcstok(wchar_t *s, const wchar_t *delim, wchar_t **save);
wchar_t *wcsdup(const wchar_t *s);
int      wcscasecmp(const wchar_t *a, const wchar_t *b);
int      wcsncasecmp(const wchar_t *a, const wchar_t *b, size_t n);

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
int      wmemcmp(const wchar_t *a, const wchar_t *b, size_t n);
wchar_t *wmemcpy(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t n);
wchar_t *wmemset(wchar_t *dst, wchar_t c, size_t n);

long               wcstol(const wchar_t *s, wchar_t **end, int base);
unsigned long      wcstoul(const wchar_t *s, wchar_t **end, int base);
long long          wcstoll(const wchar_t *s, wchar_t **end, int base);
unsigned long long wcstoull(const wchar_t *s, wchar_t **end, int base);
double             wcstod(const wchar_t *s, wchar_t **end);
float              wcstof(const wchar_t *s, wchar_t **end);
long double        wcstold(const wchar_t *s, wchar_t **end);

wint_t btowc(int c);
int    wctob(wint_t c);
int    mbsinit(const mbstate_t *ps);
size_t mbrlen(const char *s, size_t n, mbstate_t *ps);
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
size_t mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps);
size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps);
size_t mbsnrtowcs(wchar_t *dst, const char **src, size_t nms, size_t len, mbstate_t *ps);
size_t wcsnrtombs(char *dst, const wchar_t **src, size_t nwc, size_t len, mbstate_t *ps);

int    wcwidth(wchar_t c);
int    wcswidth(const wchar_t *s, size_t n);
size_t wcsftime(wchar_t *s, size_t max, const wchar_t *fmt, const struct tm *tm);

// wide stdio, utf-8 underneath. %ls and %lc arent understood by the printf ones
wint_t   fgetwc(FILE *f);
wint_t   getwc(FILE *f);
wint_t   getwchar(void);
wint_t   fputwc(wchar_t c, FILE *f);
wint_t   putwc(wchar_t c, FILE *f);
wint_t   putwchar(wchar_t c);
wint_t   ungetwc(wint_t c, FILE *f);
wchar_t *fgetws(wchar_t *s, int n, FILE *f);
int      fputws(const wchar_t *s, FILE *f);
int      fwide(FILE *f, int mode);

int swprintf(wchar_t *s, size_t n, const wchar_t *fmt, ...);
int vswprintf(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap);
int fwprintf(FILE *f, const wchar_t *fmt, ...);
int vfwprintf(FILE *f, const wchar_t *fmt, va_list ap);
int wprintf(const wchar_t *fmt, ...);
int vwprintf(const wchar_t *fmt, va_list ap);
int swscanf(const wchar_t *s, const wchar_t *fmt, ...);
int vswscanf(const wchar_t *s, const wchar_t *fmt, va_list ap);
int fwscanf(FILE *f, const wchar_t *fmt, ...);
int vfwscanf(FILE *f, const wchar_t *fmt, va_list ap);
int wscanf(const wchar_t *fmt, ...);
int vwscanf(const wchar_t *fmt, va_list ap);

#ifdef __cplusplus
}
#endif
