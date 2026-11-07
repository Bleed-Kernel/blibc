#include <locale.h>
#include <string.h>

#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <langinfo.h>
#include <nl_types.h>
#include "thread/thread.h"

static struct lconv c_lconv = {
    .decimal_point      = ".",
    .thousands_sep      = "",
    .grouping           = "",
    .int_curr_symbol    = "",
    .currency_symbol    = "",
    .mon_decimal_point  = "",
    .mon_thousands_sep  = "",
    .mon_grouping       = "",
    .positive_sign      = "",
    .negative_sign      = "",
    .int_frac_digits    = CHAR_MAX,
    .frac_digits        = CHAR_MAX,
    .p_cs_precedes      = CHAR_MAX,
    .p_sep_by_space     = CHAR_MAX,
    .n_cs_precedes      = CHAR_MAX,
    .n_sep_by_space     = CHAR_MAX,
    .p_sign_posn        = CHAR_MAX,
    .n_sign_posn        = CHAR_MAX,
    .int_p_cs_precedes  = CHAR_MAX,
    .int_p_sep_by_space = CHAR_MAX,
    .int_n_cs_precedes  = CHAR_MAX,
    .int_n_sep_by_space = CHAR_MAX,
    .int_p_sign_posn    = CHAR_MAX,
    .int_n_sign_posn    = CHAR_MAX,
};

// the one locale there is. every locale_t points at it
struct __locale_struct {
    int utf8;
};

static struct __locale_struct the_locale = { 1 };

// "C", "POSIX", "" and anything naming utf-8 are all the same thing here regardless
static int name_ok(const char *name) {
    if (!*name || !strcmp(name, "C") || !strcmp(name, "POSIX"))
        return 1;
    return strstr(name, "UTF-8") || strstr(name, "utf8") || strstr(name, "UTF8") || strstr(name, "utf-8");
}

locale_t newlocale(int mask, const char *name, locale_t base) {
    (void)base;
    if (mask & ~LC_ALL_MASK) {
        errno = EINVAL;
        return NULL;
    }
    if (!name || !name_ok(name)) {
        errno = ENOENT;
        return NULL;
    }
    return &the_locale;
}

locale_t duplocale(locale_t loc) {
    (void)loc;
    return &the_locale;
}

void freelocale(locale_t loc) {
    (void)loc;
}

// per thread, NULL in the thread is the global one
locale_t uselocale(locale_t loc) {
    struct __pthread *self = thread_self();
    locale_t old = self->locale ? (locale_t)self->locale : LC_GLOBAL_LOCALE;
    if (loc)
        self->locale = loc == LC_GLOBAL_LOCALE ? NULL : (void *)loc;
    return old;
}

static const char *const lang_days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char *const lang_abdays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *const lang_months[] = { "January", "February", "March", "April", "May", "June", "July",
                                           "August", "September", "October", "November", "December" };
static const char *const lang_abmonths[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

char *nl_langinfo(nl_item item) {
    if (item >= ABDAY_1 && item <= ABDAY_7) return (char *)lang_abdays[item - ABDAY_1];
    if (item >= DAY_1 && item <= DAY_7)     return (char *)lang_days[item - DAY_1];
    if (item >= ABMON_1 && item <= ABMON_12) return (char *)lang_abmonths[item - ABMON_1];
    if (item >= MON_1 && item <= MON_12)    return (char *)lang_months[item - MON_1];
    switch (item) {
        case T_FMT_AMPM: return "%I:%M:%S %p";
        case YESEXPR:   return "^[yY]";
        case NOEXPR:    return "^[nN]";
        case CODESET:   return "UTF-8";
        case RADIXCHAR: return ".";
        case THOUSEP:   return "";
        case D_T_FMT:   return "%a %b %e %H:%M:%S %Y";
        case D_FMT:     return "%m/%d/%y";
        case T_FMT:     return "%H:%M:%S";
        case AM_STR:    return "AM";
        case PM_STR:    return "PM";
        default:        return "";
    }
}

char *nl_langinfo_l(nl_item item, locale_t loc) {
    (void)loc;
    return nl_langinfo(item);
}

char *setlocale(int category, const char *locale) {
    if (category < LC_CTYPE || category > LC_ALL)
        return NULL;

    if (!locale || !*locale)
        return "C";

    if (strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0)
        return "C";
    if (name_ok(locale))
        return "C.UTF-8";
    return NULL;
}

struct lconv *localeconv(void) {
    return &c_lconv;
}

nl_catd catopen(const char *name, int flag) {
    (void)name;
    (void)flag;
    errno = ENOENT;
    return (nl_catd)-1;
}

char *catgets(nl_catd cat, int set, int msg, const char *def) {
    (void)cat;
    (void)set;
    (void)msg;
    return (char *)def;
}

int catclose(nl_catd cat) {
    (void)cat;
    return 0;
}
