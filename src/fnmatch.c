#include <fnmatch.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static int fold(int c, int flags) {
    return (flags & FNM_CASEFOLD) ? tolower(c) : c;
}

static bool hidden_start(const char *s, const char *start, int flags) {
    if (!(flags & FNM_PERIOD) || *s != '.')
        return false;
    return s == start || ((flags & FNM_PATHNAME) && s[-1] == '/');
}

// p sits just past '[', returns the char after ']' or NULL if c isnt in the set
static const char *match_bracket(const char *p, int c, int flags) {
    bool negate = false;
    bool found = false;

    if (*p == '!' || *p == '^') {
        negate = true;
        p++;
    }

    // a ] straight after the [ is part of the set
    const char *first = p;
    while (*p && (*p != ']' || p == first)) {
        int lo = (unsigned char)*p;
        if (lo == '\\' && !(flags & FNM_NOESCAPE) && p[1])
            lo = (unsigned char)*++p;
        p++;

        int hi = lo;
        if (*p == '-' && p[1] && p[1] != ']') {
            p++;
            hi = (unsigned char)*p;
            if (hi == '\\' && !(flags & FNM_NOESCAPE) && p[1])
                hi = (unsigned char)*++p;
            p++;
        }

        if (fold(c, flags) >= fold(lo, flags) && fold(c, flags) <= fold(hi, flags))
            found = true;
    }

    if (*p != ']')
        return NULL;
    return found != negate ? p + 1 : NULL;
}

static int match(const char *p, const char *s, const char *start, int flags) {
    for (;;) {
        switch (*p) {
        case '\0':
            if (!*s || ((flags & FNM_LEADING_DIR) && *s == '/'))
                return 0;
            return FNM_NOMATCH;

        case '?':
            if (!*s || ((flags & FNM_PATHNAME) && *s == '/') || hidden_start(s, start, flags))
                return FNM_NOMATCH;
            p++;
            s++;
            break;

        case '*':
            while (*p == '*')
                p++;
            if (hidden_start(s, start, flags))
                return FNM_NOMATCH;
            if (!*p)
                return ((flags & FNM_PATHNAME) && !(flags & FNM_LEADING_DIR) && strchr(s, '/')) ? FNM_NOMATCH : 0;

            for (; *s; s++) {
                if (match(p, s, start, flags) == 0)
                    return 0;
                if ((flags & FNM_PATHNAME) && *s == '/')
                    return FNM_NOMATCH;
            }
            return match(p, s, start, flags);

        case '[': {
            if (!*s || ((flags & FNM_PATHNAME) && *s == '/') || hidden_start(s, start, flags))
                return FNM_NOMATCH;
            const char *next = match_bracket(p + 1, (unsigned char)*s, flags);
            if (!next)
                return FNM_NOMATCH;
            p = next;
            s++;
            break;
        }

        case '\\':
            if (!(flags & FNM_NOESCAPE) && p[1])
                p++;
            // fall through
        default:
            if (fold((unsigned char)*p, flags) != fold((unsigned char)*s, flags))
                return FNM_NOMATCH;
            p++;
            s++;
            break;
        }
    }
}

int fnmatch(const char *pattern, const char *string, int flags) {
    return match(pattern, string, string, flags);
}
