#include <stdint.h>
#include <string.h>
#include "floatfmt.h"

// a double is a 53 bit whole number times a power of two, which always has an exact decimal

#define LIMB        1000000000u
#define LIMBS_MAX   300

typedef struct {
    uint32_t limb[LIMBS_MAX];
    int      count;
} big_t;

static void big_set(big_t *b, uint64_t v) {
    b->count = 0;
    do {
        b->limb[b->count++] = (uint32_t)(v % LIMB);
        v /= LIMB;
    } while (v);
}

static void big_mul(big_t *b, uint32_t by) {
    uint64_t carry = 0;
    for (int i = 0; i < b->count; i++) {
        uint64_t v = (uint64_t)b->limb[i] * by + carry;
        b->limb[i] = (uint32_t)(v % LIMB);
        carry      = v / LIMB;
    }
    while (carry && b->count < LIMBS_MAX) {
        b->limb[b->count++] = (uint32_t)(carry % LIMB);
        carry /= LIMB;
    }
}

// every digit of the number, most significant first, no leading zeros
static int exact_digits(double v, char *digits, int *point) {
    union { double d; uint64_t u; } bits = { v };
    int      e    = (int)((bits.u >> 52) & 0x7ff);
    uint64_t mant = bits.u & 0xfffffffffffffULL;

    if (e == 0) {
        e = 1;
    } else {
        mant |= 1ULL << 52;
    }
    e -= 1075;

    static big_t b;
    big_set(&b, mant);
    int shift = 0;
    if (e > 0) {
        for (; e >= 29; e -= 29)
            big_mul(&b, 1u << 29);
        if (e)
            big_mul(&b, 1u << e);
    } else if (e < 0) {
        shift = -e;
        int k = -e;
        for (; k >= 13; k -= 13)
            big_mul(&b, 1220703125u);
        uint32_t rest = 1;
        while (k--)
            rest *= 5;
        if (rest > 1)
            big_mul(&b, rest);
    }

    // out as decimal text
    int n = 0;
    char top[16];
    int  tn = 0;
    uint32_t t = b.limb[b.count - 1];
    do {
        top[tn++] = (char)('0' + t % 10);
        t /= 10;
    } while (t);
    while (tn)
        digits[n++] = top[--tn];
    for (int i = b.count - 2; i >= 0; i--) {
        uint32_t l = b.limb[i];
        for (int d = 8; d >= 0; d--) {
            digits[n + d] = (char)('0' + l % 10);
            l /= 10;
        }
        n += 9;
    }

    int lead = 0;
    while (lead < n - 1 && digits[lead] == '0')
        lead++;
    if (lead) {
        memmove(digits, digits + lead, (size_t)(n - lead));
        n -= lead;
    }
    *point = n - shift;
    while (n > 1 && digits[n - 1] == '0')
        n--;
    if (n == 1 && digits[0] == '0')
        *point = 1;
    return n;
}

static int round_at(char *digits, int *n, int keep) {
    if (keep >= *n)
        return 0;
    if (keep < 0) {
        *n = 0;
        return 0;
    }

    int up = 0;
    char next = digits[keep];
    if (next > '5') {
        up = 1;
    } else if (next == '5') {
        for (int i = keep + 1; i < *n; i++) {
            if (digits[i] != '0')
                up = 1;
        }
        if (!up)
            up = keep > 0 ? (digits[keep - 1] - '0') & 1 : 0;
    }
    *n = keep;
    if (!up)
        return 0;

    for (int i = keep - 1; i >= 0; i--) {
        if (digits[i] != '9') {
            digits[i]++;
            return 0;
        }
        digits[i] = '0';
    }
    // all nines, theres a new digit in front
    memmove(digits + 1, digits, (size_t)*n);
    digits[0] = '1';
    (*n)++;
    return 1;
}

typedef struct {
    char *s;
    int   cap, len;
} out_t;

static void put(out_t *o, char c) {
    if (o->len < o->cap)
        o->s[o->len] = c;
    o->len++;
}

static char digit_at(const char *digits, int n, int i) {
    return i >= 0 && i < n ? digits[i] : '0';
}

static void fixed(out_t *o, const char *digits, int n, int point, int prec, int alt) {
    if (point <= 0) {
        put(o, '0');
    } else {
        for (int i = 0; i < point; i++)
            put(o, digit_at(digits, n, i));
    }
    if (prec || alt)
        put(o, '.');
    for (int i = 0; i < prec; i++)
        put(o, digit_at(digits, n, point + i));
}

static void sci(out_t *o, const char *digits, int n, int exp10, int prec, int alt, int upper) {
    put(o, digit_at(digits, n, 0));
    if (prec || alt)
        put(o, '.');
    for (int i = 1; i <= prec; i++)
        put(o, digit_at(digits, n, i));
    put(o, upper ? 'E' : 'e');
    put(o, exp10 < 0 ? '-' : '+');
    int e = exp10 < 0 ? -exp10 : exp10;
    char eb[8];
    int  en = 0;
    do {
        eb[en++] = (char)('0' + e % 10);
        e /= 10;
    } while (e);
    if (en < 2)
        eb[en++] = '0';
    while (en)
        put(o, eb[--en]);
}

// %g takes trailing zeros off
static void trim(out_t *o, int from) {
    int end = o->len < o->cap ? o->len : o->cap;
    int dot = -1, exp_at = end;
    for (int i = from; i < end; i++) {
        if (o->s[i] == '.') dot = i;
        if (o->s[i] == 'e' || o->s[i] == 'E') exp_at = i;
    }
    if (dot < 0)
        return;
    int cut = exp_at;
    while (cut > dot + 1 && o->s[cut - 1] == '0')
        cut--;
    if (cut == dot + 1)
        cut = dot;
    memmove(o->s + cut, o->s + exp_at, (size_t)(end - exp_at));
    o->len -= exp_at - cut;
}

int floatfmt(char *buf, int cap, double v, char conv, int prec, int alt) {
    out_t o = { buf, cap, 0 };
    int upper = conv == 'F' || conv == 'E' || conv == 'G';
    union { double d; uint64_t u; } bits = { v };

    if (((bits.u >> 52) & 0x7ff) == 0x7ff) {
        const char *word = (bits.u & 0xfffffffffffffULL) ? (upper ? "NAN" : "nan") : (upper ? "INF" : "inf");
        for (; *word; word++)
            put(&o, *word);
        return -(o.len < cap ? o.len : cap);
    }
    bits.u &= ~(1ULL << 63);    // the sign is written by whoever called
    v = bits.d;

    static char digits[2700];
    int point, n = exact_digits(v, digits, &point);
    int zero = n == 1 && digits[0] == '0';

    switch (conv) {
    case 'f':
    case 'F':
        if (round_at(digits, &n, point + prec))
            point++;
        fixed(&o, digits, n, point, prec, alt);
        break;

    case 'e':
    case 'E': {
        int exp10 = zero ? 0 : point - 1;
        if (round_at(digits, &n, prec + 1))
            exp10++;
        sci(&o, digits, n, exp10, prec, alt, upper);
        break;
    }

    default: {          // g
        int p     = prec ? prec : 1;
        int exp10 = zero ? 0 : point - 1;
        if (round_at(digits, &n, p))
            exp10++, point++;
        if (exp10 < -4 || exp10 >= p)
            sci(&o, digits, n, exp10, p - 1, alt, upper);
        else
            fixed(&o, digits, n, point, p - 1 - exp10, alt);
        if (!alt)
            trim(&o, 0);
        break;
    }
    }
    return o.len < cap ? o.len : cap;
}
