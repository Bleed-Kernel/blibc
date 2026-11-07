#include <wctype.h>
#include <ctype.h>
#include <string.h>

static int uni_space(wint_t c) {
    return c == 0x1680 || (c >= 0x2000 && c <= 0x2006) || (c >= 0x2008 && c <= 0x200A) ||
           c == 0x2028 || c == 0x2029 || c == 0x205F || c == 0x3000;
}

static int uni_punct(wint_t c) {
    return (c >= 0x2010 && c <= 0x2027) || (c >= 0x2030 && c <= 0x205E) || (c >= 0x2190 && c <= 0x23FF) ||
           (c >= 0x2500 && c <= 0x27BF) || (c >= 0x3001 && c <= 0x3003) || (c >= 0x3008 && c <= 0x3011) ||
           (c >= 0xFF01 && c <= 0xFF0F) || (c >= 0x1F300 && c <= 0x1FAFF) || (c >= 0x20A0 && c <= 0x20CF);
}

int iswspace(wint_t c) {
    if (c < 0x80)
        return isspace((int)c);
    return c == 0x85 || uni_space(c);
}

int iswblank(wint_t c) {
    return c == ' ' || c == '\t' || (c >= 0x80 && uni_space(c) && c != 0x2028 && c != 0x2029);
}

int iswcntrl(wint_t c) {
    return c < 0x20 || (c >= 0x7F && c < 0xA0) || c == 0x2028 || c == 0x2029;
}

int iswdigit(wint_t c) {
    return c >= '0' && c <= '9';
}

int iswxdigit(wint_t c) {
    return c < 0x80 && isxdigit((int)c);
}

int iswupper(wint_t c) {
    if (c < 0x80)
        return isupper((int)c);
    return (c >= 0xC0 && c <= 0xDE && c != 0xD7) || (c >= 0x391 && c <= 0x3AB) || (c >= 0x400 && c <= 0x42F) ||
           (c >= 0x100 && c <= 0x17F && !(c & 1));
}

int iswlower(wint_t c) {
    if (c < 0x80)
        return islower((int)c);
    return (c >= 0xDF && c <= 0xFF && c != 0xF7) || (c >= 0x3B1 && c <= 0x3CB) || (c >= 0x430 && c <= 0x45F) ||
           (c >= 0x100 && c <= 0x17F && (c & 1));
}

int iswalpha(wint_t c) {
    if (c < 0x80)
        return isalpha((int)c);
    if (c < 0xA0 || c > 0x10FFFF)
        return 0;
    if (c < 0x100)
        return c == 0xAA || c == 0xB5 || c == 0xBA || (c >= 0xC0 && c != 0xD7 && c != 0xF7);
    if (c >= 0x300 && c <= 0x36F)
        return 0;
    return !uni_space(c) && !uni_punct(c) && !(c >= 0xD800 && c <= 0xDFFF) && !(c >= 0xE000 && c <= 0xF8FF) &&
           c != 0xFFFE && c != 0xFFFF;
}

int iswalnum(wint_t c) {
    return iswalpha(c) || iswdigit(c);
}

int iswpunct(wint_t c) {
    if (c < 0x80)
        return ispunct((int)c);
    if (c >= 0xA1 && c <= 0xBF)
        return c != 0xAA && c != 0xB5 && c != 0xBA;
    return c == 0xD7 || c == 0xF7 || uni_punct(c);
}

int iswgraph(wint_t c) {
    return iswalnum(c) || iswpunct(c) || (c >= 0x300 && c <= 0x36F);
}

int iswprint(wint_t c) {
    return iswgraph(c) || c == ' ' || (c >= 0x80 && uni_space(c) && c != 0x2028 && c != 0x2029);
}

wint_t towupper(wint_t c) {
    if (c < 0x80)
        return (wint_t)toupper((int)c);
    if ((c >= 0xE0 && c <= 0xFE && c != 0xF7) || (c >= 0x3B1 && c <= 0x3CB && c != 0x3C2) || (c >= 0x430 && c <= 0x44F))
        return c - 0x20;
    if (c >= 0x450 && c <= 0x45F)
        return c - 0x50;
    if (c == 0xFF)
        return 0x178;
    if (c >= 0x100 && c <= 0x17F && (c & 1))
        return c - 1;
    return c;
}

wint_t towlower(wint_t c) {
    if (c < 0x80)
        return (wint_t)tolower((int)c);
    if ((c >= 0xC0 && c <= 0xDE && c != 0xD7) || (c >= 0x391 && c <= 0x3AB) || (c >= 0x410 && c <= 0x42F))
        return c + 0x20;
    if (c >= 0x400 && c <= 0x40F)
        return c + 0x50;
    if (c == 0x178)
        return 0xFF;
    if (c >= 0x100 && c <= 0x17F && !(c & 1) && c != 0x130)
        return c + 1;
    return c;
}

// wctype names in the order wctype() hands back 1 based numbers
static const char *const class_names[] = {
    "alnum", "alpha", "blank", "cntrl", "digit", "graph", "lower", "print", "punct", "space", "upper", "xdigit",
};

wctype_t wctype(const char *name) {
    for (size_t i = 0; i < sizeof(class_names) / sizeof(class_names[0]); i++) {
        if (!strcmp(name, class_names[i]))
            return i + 1;
    }
    return 0;
}

int iswctype(wint_t c, wctype_t type) {
    switch (type) {
        case 1:  return iswalnum(c);
        case 2:  return iswalpha(c);
        case 3:  return iswblank(c);
        case 4:  return iswcntrl(c);
        case 5:  return iswdigit(c);
        case 6:  return iswgraph(c);
        case 7:  return iswlower(c);
        case 8:  return iswprint(c);
        case 9:  return iswpunct(c);
        case 10: return iswspace(c);
        case 11: return iswupper(c);
        case 12: return iswxdigit(c);
        default: return 0;
    }
}

static const int trans_lower = 1;
static const int trans_upper = 2;

wctrans_t wctrans(const char *name) {
    if (!strcmp(name, "tolower"))
        return &trans_lower;
    if (!strcmp(name, "toupper"))
        return &trans_upper;
    return NULL;
}

wint_t towctrans(wint_t c, wctrans_t t) {
    if (t == &trans_lower)
        return towlower(c);
    if (t == &trans_upper)
        return towupper(c);
    return c;
}
