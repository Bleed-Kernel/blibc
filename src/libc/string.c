#include <libc/stdint.h>
#include <libc/stdlib.h>
#include <libc/ctype.h>
#include <libc/string.h>
#include <libc/strings.h>
#include <stddef.h>

/// @brief set memory at destination
/// @param s destination
/// @param c byte value
/// @param n size
/// @return s
void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

/// @brief copy memory from source to destination
/// @param dest destination
/// @param src source
/// @param n size
/// @return dest
void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}


/// @brief move memory from destination to source
/// @param dest destination
/// @param src source
/// @param n size to evaluate
/// @return void
void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

/// @brief compare two blocks of memory
/// @param s1 block 1
/// @param s2 block 2
/// @param n size to evaluate
/// @return result
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

/// @brief gets the length of a string
/// @param string string to evaluate
/// @return uint32 length
size_t strlen(const char *string){
    if (string == NULL) return 0;

    size_t length = 0;
    while (*string != '\0'){
        length++;
        string++;
    }
    return length;
}

#include <stdint.h>

/// @brief copy a string
/// @param dest destination buffer
/// @param src source string
/// @return dest
char *strcpy(char *restrict dest, const char *restrict src) {
    char *d = dest;
    while (*src != '\0') {
        *d++ = *src++;
    }
    *d = '\0';
    return dest;
}

/// @brief copy at most n chars of src into dest
/// @param dest destination
/// @param src source
/// @param n maximum chars
/// @return dest
char *strncpy(char *restrict dest, const char *restrict src, size_t n) {
    char *d = dest;

    for (size_t i = 0; i < n; i++) {
        if (src[i] != '\0') {
            d[i] = src[i];
        } else {
            for (; i < n; i++) {
                d[i] = '\0';
            }
            return dest;
        }
    }

    return dest;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    char ch = (char)c;

    while (*s != '\0') {
        if (*s == ch) {
            last = s;
        }
        s++;
    }

    if (ch == '\0') {
        return (char *)s;
    }

    return (char *)last;
}

/// @brief find first occurrence of needle in haystack
/// @param haystack string to search
/// @param needle substring to find
/// @return pointer to first match or NULL
char *strstr(const char *haystack, const char *needle) {
    if (*needle == '\0') {
        return (char *)haystack;
    }

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (*n == '\0') {
            return (char *)haystack;
        }

        haystack++;
    }

    return NULL;
}

/// @brief concatenate src onto end of dest
/// @param dest destination string buffer
/// @param src source string to append
/// @return dest
char *strcat(char *restrict dest, const char *restrict src) {
    char *d = dest;

    while (*d != '\0') d++;

    while (*src != '\0') {
        *d++ = *src++;
    }

    *d = '\0';
    return dest;
}

char *strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s) + 1;
    char *dup = (char *)malloc(len);

    if (dup == NULL) {
        return NULL;
    }

    memmove(dup, s, len);
    return dup;
}

char *strndup(const char *s, size_t n) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = 0;
    while (len < n && s[len] != '\0') {
        len++;
    }

    char *dup = (char *)malloc(len + 1);
    if (dup == NULL) {
        return NULL;
    }

    memmove(dup, s, len);
    dup[len] = '\0';
    return dup;
}

/// @brief compare two strings
/// @param s1 string 1
/// @param s2 string 2
/// @return <0, 0, >0
int strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        if (*s1 != *s2) {
            return (*s1 < *s2) ? -1 : 1;
        }
        s1++;
        s2++;
    }

    if (*s1 == *s2) return 0;
    return (*s1 < *s2) ? -1 : 1;
}

/// @brief compare at most n bytes of strings
/// @param s1 string 1
/// @param s2 string 2
/// @param n number of chars to compare
/// @return <0, 0, >0
int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (s1[i] < s2[i]) ? -1 : 1;
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

/// @brief find first occurrence of c in string s
/// @param s string
/// @param c character
/// @return pointer to char or NULL
char *strchr(const char *s, int c) {
    while (*s != '\0') {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

/// @brief string concatonation of a size
/// @param dest destination
/// @param src start concat
/// @return char array
char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    
    while (*d) {
        d++;
    }

    size_t i = 0;
    while (i < n && src[i]) {
        d[i] = src[i];
        i++;
    }

    d[i] = '\0';
    return dest;
}

/// @brief tokenize a string (not thread-safe)
/// @param s input string or NULL to continue
/// @param delim delimiter characters
/// @return next token or NULL
char *strtok(char *restrict s, const char *restrict delim) {
    static char *static_ptr = NULL;
    return strtok_r(s, delim, &static_ptr);
}

/// @brief reentrant tokenizer
/// @param s string to tokenize or NULL
/// @param delim delimiter chars
/// @param save pointer to save state
/// @return next token
char *strtok_r(char *restrict s, const char *restrict delim, char **restrict save) {
    if (s == NULL) {
        s = *save;
    }

    if (s == NULL) {
        return NULL;
    }

    while (*s != '\0') {
        const char *d = delim;
        int found = 0;

        while (*d != '\0') {
            if (*s == *d) {
                found = 1;
                break;
            }
            d++;
        }

        if (!found) break;
        s++;
    }

    if (*s == '\0') {
        *save = NULL;
        return NULL;
    }

    char *token_start = s;

    while (*s != '\0') {
        const char *d = delim;
        while (*d != '\0') {
            if (*s == *d) {
                *s = '\0';
                *save = s + 1;
                return token_start;
            }
            d++;
        }
        s++;
    }

    *save = NULL;
    return token_start;
}

int bcmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1, s2, n);
}

void bcopy(const void *src, void *dest, size_t n) {
    memmove(dest, src, n);
}

void bzero(void *s, size_t n) {
    memset(s, 0, n);
}

char *index(const char *s, int c) {
    return strchr(s, c);
}

char *rindex(const char *s, int c) {
    return strrchr(s, c);
}

int strcasecmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) {
            return (c1 < c2) ? -1 : 1;
        }
        s1++;
        s2++;
    }

    if (*s1 == *s2) {
        return 0;
    }

    return ((unsigned char)*s1 < (unsigned char)*s2) ? -1 : 1;
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int c1 = tolower((unsigned char)s1[i]);
        int c2 = tolower((unsigned char)s2[i]);

        if (c1 != c2) {
            return (c1 < c2) ? -1 : 1;
        }

        if (s1[i] == '\0') {
            return 0;
        }
    }

    return 0;
}

int ffs(int i) {
    if (i == 0) {
        return 0;
    }

    int bit = 1;
    while ((i & 1) == 0) {
        i >>= 1;
        bit++;
    }
    return bit;
}
