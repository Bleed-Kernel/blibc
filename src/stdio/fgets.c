#include <libc/stdio.h>

char *fgets(char *s, int size, FILE *stream) {
    if (!s || size <= 0 || !stream) {
        return NULL;
    }

    if (size == 1) {
        s[0] = '\0';
        return s;
    }

    int i = 0;
    while (i < size - 1) {
        char ch;
        if (fread(&ch, 1, 1, stream) != 1) {
            break;
        }

        s[i++] = ch;
        if (ch == '\n') {
            break;
        }
    }

    if (i == 0) {
        return NULL;
    }

    s[i] = '\0';
    return s;
}
