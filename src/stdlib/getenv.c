#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>

char *getenv(const char *name) {
    if (!name || !*name || !environ)
        return NULL;

    size_t name_len = strlen(name);
    for (char **env = environ; *env; env++) {
        if (strncmp(*env, name, name_len) == 0 && (*env)[name_len] == '=')
            return *env + name_len + 1;
    }

    return NULL;
}
