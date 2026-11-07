#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#define LINKS_MAX   40

char *realpath(const char *path, char *resolved) {
    if (!path || !*path) {
        errno = path ? ENOENT : EINVAL;
        return NULL;
    }

    // todo is whats left to walk, out is whats been walked so far
    char todo[PATH_MAX], out[PATH_MAX] = "";
    size_t n = 0;
    if (strlen(path) >= sizeof(todo)) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    if (path[0] == '/') {
        strcpy(todo, path);
    } else {
        if (!getcwd(out, sizeof(out)))
            return NULL;
        n = strlen(out);
        if (n == 1)
            n = 0;              // just "/"
        strcpy(todo, path);
    }

    int links = 0;
    char *p = todo;
    while (*p) {
        while (*p == '/')
            p++;
        if (!*p)
            break;
        char *end = strchr(p, '/');
        size_t plen = end ? (size_t)(end - p) : strlen(p);

        if (plen == 1 && p[0] == '.') {
            p += plen;
            continue;
        }
        if (plen == 2 && p[0] == '.' && p[1] == '.') {
            while (n > 0 && out[n - 1] != '/')
                n--;
            if (n > 0)
                n--;
            out[n] = '\0';
            p += plen;
            continue;
        }

        if (n + 1 + plen >= sizeof(out)) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        size_t at = n;
        out[n++] = '/';
        memcpy(out + n, p, plen);
        n += plen;
        out[n] = '\0';
        p += plen;

        struct stat st;
        if (lstat(out, &st) < 0)
            return NULL;
        if (!S_ISLNK(st.st_mode))
            continue;

        if (++links > LINKS_MAX) {
            errno = ELOOP;
            return NULL;
        }
        char target[PATH_MAX];
        ssize_t tlen = readlink(out, target, sizeof(target) - 1);
        if (tlen < 0)
            return NULL;
        target[tlen] = '\0';

        // the target then whatever was after the link
        if ((size_t)tlen + 1 + strlen(p) >= sizeof(todo)) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        char rest[PATH_MAX];
        strcpy(rest, p);
        strcpy(todo, target);
        strcat(todo, "/");
        strcat(todo, rest);
        p = todo;

        n = target[0] == '/' ? 0 : at;
        out[n] = '\0';
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
