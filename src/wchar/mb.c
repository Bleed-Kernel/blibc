#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static size_t ilseq(mbstate_t *ps) {
    ps->state = 0;
    errno = EILSEQ;
    return (size_t)-1;
}

size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps) {
    static mbstate_t internal;
    if (!ps)
        ps = &internal;
    if (!s) {
        pwc = NULL;
        s   = "";
        n   = 1;
    }
    if (!n)
        return (size_t)-2;

    unsigned total = ps->state >> 28;
    unsigned need  = (ps->state >> 24) & 0xF;
    unsigned val   = ps->state & 0xFFFFFF;
    size_t   i     = 0;

    if (!need) {
        unsigned char c = (unsigned char)s[i++];
        if (c < 0x80) {
            if (pwc)
                *pwc = c;
            ps->state = 0;
            return c ? 1 : 0;
        }
        if (c < 0xC2 || c > 0xF4)
            return ilseq(ps);
        if (c < 0xE0) {
            total = 2;
            val   = c & 0x1F;
        } else if (c < 0xF0) {
            total = 3;
            val   = c & 0x0F;
        } else {
            total = 4;
            val   = c & 0x07;
        }
        need = total - 1;
    }

    while (need && i < n) {
        unsigned char c = (unsigned char)s[i++];
        if ((c & 0xC0) != 0x80)
            return ilseq(ps);
        val = (val << 6) | (c & 0x3F);
        need--;
    }
    if (need) {
        ps->state = (total << 28) | (need << 24) | val;
        return (size_t)-2;
    }

    unsigned min = total == 2 ? 0x80 : total == 3 ? 0x800 : 0x10000;
    if (val < min || val > 0x10FFFF || (val >= 0xD800 && val <= 0xDFFF))
        return ilseq(ps);

    if (pwc)
        *pwc = (wchar_t)val;
    ps->state = 0;
    return i;
}

size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps) {
    (void)ps;
    if (!s)
        return 1;
    uint32_t c = (uint32_t)wc;
    if (c < 0x80) {
        s[0] = (char)c;
        return 1;
    }
    if (c < 0x800) {
        s[0] = (char)(0xC0 | (c >> 6));
        s[1] = (char)(0x80 | (c & 0x3F));
        return 2;
    }
    if ((c >= 0xD800 && c <= 0xDFFF) || c > 0x10FFFF) {
        errno = EILSEQ;
        return (size_t)-1;
    }
    if (c < 0x10000) {
        s[0] = (char)(0xE0 | (c >> 12));
        s[1] = (char)(0x80 | ((c >> 6) & 0x3F));
        s[2] = (char)(0x80 | (c & 0x3F));
        return 3;
    }
    s[0] = (char)(0xF0 | (c >> 18));
    s[1] = (char)(0x80 | ((c >> 12) & 0x3F));
    s[2] = (char)(0x80 | ((c >> 6) & 0x3F));
    s[3] = (char)(0x80 | (c & 0x3F));
    return 4;
}

size_t mbrlen(const char *s, size_t n, mbstate_t *ps) {
    static mbstate_t internal;
    return mbrtowc(NULL, s, n, ps ? ps : &internal);
}

int mbsinit(const mbstate_t *ps) {
    return !ps || !ps->state;
}

wint_t btowc(int c) {
    return c >= 0 && c < 0x80 ? (wint_t)c : WEOF;
}

int wctob(wint_t c) {
    return c < 0x80 ? (int)c : EOF;
}

size_t mbsnrtowcs(wchar_t *dst, const char **src, size_t nms, size_t len, mbstate_t *ps) {
    static mbstate_t internal;
    if (!ps)
        ps = &internal;

    const char *s     = *src;
    size_t      count = 0;
    while (nms && (!dst || count < len)) {
        wchar_t wc;
        size_t  r = mbrtowc(&wc, s, nms, ps);
        if (r == (size_t)-1) {
            if (dst)
                *src = s;
            return (size_t)-1;
        }
        if (r == (size_t)-2) {
            s  += nms;
            nms = 0;
            break;
        }
        if (r == 0) {
            if (dst) {
                dst[count] = 0;
                *src = NULL;
            }
            return count;
        }
        if (dst)
            dst[count] = wc;
        s   += r;
        nms -= r;
        count++;
    }
    if (dst)
        *src = s;
    return count;
}

size_t mbsrtowcs(wchar_t *dst, const char **src, size_t len, mbstate_t *ps) {
    return mbsnrtowcs(dst, src, (size_t)-1, len, ps);
}

size_t wcsnrtombs(char *dst, const wchar_t **src, size_t nwc, size_t len, mbstate_t *ps) {
    const wchar_t *w     = *src;
    size_t         count = 0;
    char           buf[4];

    while (nwc) {
        wchar_t wc = *w;
        size_t  r  = wcrtomb(buf, wc, ps);
        if (r == (size_t)-1) {
            if (dst)
                *src = w;
            return (size_t)-1;
        }
        if (dst) {
            if (count + r > len)
                break;
            memcpy(dst + count, buf, r);
        }
        if (!wc) {
            if (dst)
                *src = NULL;
            return count;
        }
        count += r;
        w++;
        nwc--;
    }
    if (dst)
        *src = w;
    return count;
}

size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps) {
    return wcsnrtombs(dst, src, (size_t)-1, len, ps);
}

int mbtowc(wchar_t *pwc, const char *s, size_t n) {
    if (!s)
        return 0;
    mbstate_t st = { 0 };
    size_t r = mbrtowc(pwc, s, n, &st);
    if (r == (size_t)-2) {
        errno = EILSEQ;
        return -1;
    }
    return r == (size_t)-1 ? -1 : (int)r;
}

int mblen(const char *s, size_t n) {
    return mbtowc(NULL, s, n);
}

int wctomb(char *s, wchar_t wc) {
    if (!s)
        return 0;
    size_t r = wcrtomb(s, wc, NULL);
    return r == (size_t)-1 ? -1 : (int)r;
}

size_t mbstowcs(wchar_t *dst, const char *src, size_t n) {
    mbstate_t st = { 0 };
    return mbsrtowcs(dst, &src, n, &st);
}

size_t wcstombs(char *dst, const wchar_t *src, size_t n) {
    mbstate_t st = { 0 };
    return wcsrtombs(dst, &src, n, &st);
}

// columns on a terminal: 0 for combining marks, 2 for the wide east asian ranges
int wcwidth(wchar_t c) {
    uint32_t u = (uint32_t)c;
    if (!u)
        return 0;
    if (u < 0x20 || (u >= 0x7F && u < 0xA0))
        return -1;
    if ((u >= 0x300 && u <= 0x36F) || (u >= 0x200B && u <= 0x200F) || (u >= 0xFE00 && u <= 0xFE0F) ||
        (u >= 0x1AB0 && u <= 0x1AFF) || (u >= 0x20D0 && u <= 0x20FF))
        return 0;
    if ((u >= 0x1100 && u <= 0x115F) || (u >= 0x2E80 && u <= 0xA4CF && u != 0x303F) ||
        (u >= 0xAC00 && u <= 0xD7A3) || (u >= 0xF900 && u <= 0xFAFF) || (u >= 0xFE30 && u <= 0xFE4F) ||
        (u >= 0xFF00 && u <= 0xFF60) || (u >= 0xFFE0 && u <= 0xFFE6) || (u >= 0x1F300 && u <= 0x1F64F) ||
        (u >= 0x1F900 && u <= 0x1F9FF) || (u >= 0x20000 && u <= 0x3FFFD))
        return 2;
    return 1;
}

int wcswidth(const wchar_t *s, size_t n) {
    int total = 0;
    for (; n && *s; n--, s++) {
        int w = wcwidth(*s);
        if (w < 0)
            return -1;
        total += w;
    }
    return total;
}
