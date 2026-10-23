#include <locale.h>
#include <string.h>

static struct lconv c_lconv = {
    .decimal_point     = ".",
    .thousands_sep     = "",
    .grouping          = "",
    .int_curr_symbol   = "",
    .currency_symbol   = "",
    .mon_decimal_point = "",
    .mon_thousands_sep = "",
    .mon_grouping      = "",
    .positive_sign     = "",
    .negative_sign     = "",
};

char *setlocale(int category, const char *locale) {
    if (category < LC_CTYPE || category > LC_ALL)
        return NULL;

    if (!locale || !*locale)
        return "C";

    if (strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0 ||
        strncmp(locale, "C.", 2) == 0)
        return "C";
    return NULL;
}

struct lconv *localeconv(void) {
    return &c_lconv;
}
