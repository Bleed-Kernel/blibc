#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>

size_t wcslen(const wchar_t *s) {
    const wchar_t *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

size_t wcsnlen(const wchar_t *s, size_t n) {
    size_t i = 0;
    while (i < n && s[i])
        i++;
    return i;
}

wchar_t *wcscpy(wchar_t *dst, const wchar_t *src) {
    wchar_t *d = dst;
    while ((*d++ = *src++))
        ;
    return dst;
}

wchar_t *wcsncpy(wchar_t *dst, const wchar_t *src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = 0;
    return dst;
}

wchar_t *wcscat(wchar_t *dst, const wchar_t *src) {
    wcscpy(dst + wcslen(dst), src);
    return dst;
}

wchar_t *wcsncat(wchar_t *dst, const wchar_t *src, size_t n) {
    wchar_t *d = dst + wcslen(dst);
    size_t i = 0;
    for (; i < n && src[i]; i++)
        d[i] = src[i];
    d[i] = 0;
    return dst;
}

int wcscmp(const wchar_t *a, const wchar_t *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b;
}

int wcsncmp(const wchar_t *a, const wchar_t *b, size_t n) {
    for (; n; n--, a++, b++) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
        if (!*a)
            break;
    }
    return 0;
}

// the C locale collates by code point
int wcscoll(const wchar_t *a, const wchar_t *b) {
    return wcscmp(a, b);
}

size_t wcsxfrm(wchar_t *dst, const wchar_t *src, size_t n) {
    size_t len = wcslen(src);
    if (n) {
        size_t c = len < n - 1 ? len : n - 1;
        wmemcpy(dst, src, c);
        dst[c] = 0;
    }
    return len;
}

wchar_t *wcschr(const wchar_t *s, wchar_t c) {
    for (;; s++) {
        if (*s == c)
            return (wchar_t *)s;
        if (!*s)
            return NULL;
    }
}

wchar_t *wcsrchr(const wchar_t *s, wchar_t c) {
    const wchar_t *last = NULL;
    for (;; s++) {
        if (*s == c)
            last = s;
        if (!*s)
            return (wchar_t *)last;
    }
}

wchar_t *wcsstr(const wchar_t *hay, const wchar_t *needle) {
    size_t n = wcslen(needle);
    if (!n)
        return (wchar_t *)hay;
    for (; *hay; hay++) {
        if (*hay == *needle && !wcsncmp(hay, needle, n))
            return (wchar_t *)hay;
    }
    return NULL;
}

size_t wcsspn(const wchar_t *s, const wchar_t *accept) {
    size_t i = 0;
    while (s[i] && wcschr(accept, s[i]))
        i++;
    return i;
}

size_t wcscspn(const wchar_t *s, const wchar_t *reject) {
    size_t i = 0;
    while (s[i] && !wcschr(reject, s[i]))
        i++;
    return i;
}

wchar_t *wcspbrk(const wchar_t *s, const wchar_t *accept) {
    s += wcscspn(s, accept);
    return *s ? (wchar_t *)s : NULL;
}

wchar_t *wcstok(wchar_t *s, const wchar_t *delim, wchar_t **save) {
    if (!s)
        s = *save;
    if (!s)
        return NULL;
    s += wcsspn(s, delim);
    if (!*s) {
        *save = NULL;
        return NULL;
    }
    wchar_t *end = s + wcscspn(s, delim);
    if (*end)
        *end++ = 0;
    else
        end = NULL;
    *save = end;
    return s;
}

wchar_t *wcsdup(const wchar_t *s) {
    size_t n = wcslen(s) + 1;
    wchar_t *d = malloc(n * sizeof(wchar_t));
    return d ? wmemcpy(d, s, n) : NULL;
}

int wcscasecmp(const wchar_t *a, const wchar_t *b) {
    return wcsncasecmp(a, b, (size_t)-1);
}

int wcsncasecmp(const wchar_t *a, const wchar_t *b, size_t n) {
    for (; n; n--, a++, b++) {
        wint_t x = towlower(*a), y = towlower(*b);
        if (x != y)
            return x < y ? -1 : 1;
        if (!x)
            break;
    }
    return 0;
}

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n) {
    for (; n; n--, s++) {
        if (*s == c)
            return (wchar_t *)s;
    }
    return NULL;
}

int wmemcmp(const wchar_t *a, const wchar_t *b, size_t n) {
    for (; n; n--, a++, b++) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
    }
    return 0;
}

wchar_t *wmemcpy(wchar_t *dst, const wchar_t *src, size_t n) {
    for (size_t i = 0; i < n; i++)
        dst[i] = src[i];
    return dst;
}

wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t n) {
    if (dst < src) {
        for (size_t i = 0; i < n; i++)
            dst[i] = src[i];
    } else {
        for (size_t i = n; i; i--)
            dst[i - 1] = src[i - 1];
    }
    return dst;
}

wchar_t *wmemset(wchar_t *dst, wchar_t c, size_t n) {
    for (size_t i = 0; i < n; i++)
        dst[i] = c;
    return dst;
}
