#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

// theres no links, so its just the path from the working directory with the . and .. 
char *realpath(const char *path, char *resolved) {
    if (!path || !*path) {
        errno = path ? ENOENT : EINVAL;
        return NULL;
    }

    char full[PATH_MAX];
    if (path[0] == '/') {
        if (strlen(path) >= sizeof(full)) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        strcpy(full, path);
    } else {
        if (!getcwd(full, sizeof(full)))
            return NULL;
        size_t len = strlen(full);
        if (len + 1 + strlen(path) >= sizeof(full)) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        if (full[len - 1] != '/')
            full[len++] = '/';
        strcpy(full + len, path);
    }

    char   out[PATH_MAX] = "";
    size_t n = 0;
    char  *save;
    for (char *part = strtok_r(full, "/", &save); part; part = strtok_r(NULL, "/", &save)) {
        if (!strcmp(part, "."))
            continue;
        if (!strcmp(part, "..")) {
            while (n > 0 && out[n - 1] != '/')
                n--;
            if (n > 0)
                n--;            // the slash before it
            out[n] = '\0';
            continue;
        }
        size_t plen = strlen(part);
        if (n + 1 + plen >= sizeof(out)) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        out[n++] = '/';
        memcpy(out + n, part, plen + 1);
        n += plen;
    }
    if (n == 0)
        strcpy(out, "/");

    struct stat st;
    if (stat(out, &st) < 0)
        return NULL;

    if (!resolved)
        return strdup(out);
    strcpy(resolved, out);
    return resolved;
}
