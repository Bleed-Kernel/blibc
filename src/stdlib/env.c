#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static char **owned;
static size_t owned_cap;

static size_t env_count(void) {
    size_t n = 0;
    while (environ && environ[n])
        n++;
    return n;
}

static int env_reserve(size_t want) {
    if (environ == owned && want <= owned_cap)
        return 0;

    size_t cap  = want < 16 ? 16 : want * 2;
    char **list = malloc(cap * sizeof(char *));
    if (!list) {
        errno = ENOMEM;
        return -1;
    }
    size_t n = env_count();
    memcpy(list, environ, n * sizeof(char *));
    list[n] = NULL;

    if (environ == owned)
        free(owned);
    owned     = list;
    owned_cap = cap;
    environ   = list;
    return 0;
}

static long env_find(const char *name, size_t len) {
    for (size_t i = 0; environ && environ[i]; i++) {
        if (strncmp(environ[i], name, len) == 0 && environ[i][len] == '=')
            return (long)i;
    }
    return -1;
}

// the string itself goes in the list, changing it later changes the variable
int putenv(char *string) {
    char *eq = string ? strchr(string, '=') : NULL;
    if (!eq || eq == string) {
        errno = EINVAL;
        return -1;
    }

    long at = env_find(string, (size_t)(eq - string));
    if (at >= 0) {
        if (env_reserve(env_count() + 1) < 0)
            return -1;
        environ[at] = string;
        return 0;
    }

    size_t n = env_count();
    if (env_reserve(n + 2) < 0)
        return -1;
    environ[n]     = string;
    environ[n + 1] = NULL;
    return 0;
}

int setenv(const char *name, const char *value, int overwrite) {
    if (!name || !*name || strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }
    if (!overwrite && getenv(name))
        return 0;

    size_t len = strlen(name);
    size_t vlen = value ? strlen(value) : 0;
    char  *entry = malloc(len + vlen + 2);
    if (!entry) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(entry, name, len);
    entry[len] = '=';
    memcpy(entry + len + 1, value ? value : "", vlen + 1);
    return putenv(entry);
}

int unsetenv(const char *name) {
    if (!name || !*name || strchr(name, '=')) {
        errno = EINVAL;
        return -1;
    }

    long at;
    while ((at = env_find(name, strlen(name))) >= 0) {
        if (env_reserve(env_count() + 1) < 0)
            return -1;
        for (long i = at; environ[i]; i++)
            environ[i] = environ[i + 1];
    }
    return 0;
}
