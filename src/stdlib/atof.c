#include <stdlib.h>

double atof(const char *s) {
    return s ? strtod(s, NULL) : 0.0;
}
