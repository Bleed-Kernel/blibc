#include <ctype.h>

int isascii(int c) {
    return (unsigned)c <= 0x7F;
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int islower(int c) {
    return c >= 'a' && c <= 'z';
}

int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

int isalpha(int c) {
    return islower(c) || isupper(c);
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isxdigit(int c) {
    return isdigit(c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

int isspace(int c) {
    return c == ' '  ||
           c == '\t' ||
           c == '\n' ||
           c == '\r' ||
           c == '\v' ||
           c == '\f';
}

int isblank(int c) {
    return c == ' ' || c == '\t';
}

int isprint(int c) {
    return c >= 32 && c < 127;
}

int tolower(int c) {
    if (isupper(c))
        return c + ('a' - 'A');
    return c;
}

int toupper(int c) {
    if (islower(c))
        return c - ('a' - 'A');
    return c;
}
