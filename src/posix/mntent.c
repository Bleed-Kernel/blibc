#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bleed/syscalls.h>
#include <bleed/sysinfo.h>

typedef struct mnt_cursor {
    FILE              *f;
    unsigned long      next;
    struct mnt_cursor *link;
} mnt_cursor_t;

static mnt_cursor_t *cursors;

static mnt_cursor_t *cursor_of(FILE *f) {
    for (mnt_cursor_t *c = cursors; c; c = c->link)
        if (c->f == f)
            return c;
    return NULL;
}

FILE *setmntent(const char *path, const char *mode) {
    (void)path; (void)mode;
    mnt_cursor_t *c = calloc(1, sizeof(*c));
    if (!c)
        return NULL;
    c->f = fopen("/dev/null", "r");
    if (!c->f) {
        free(c);
        return NULL;
    }
    c->link = cursors;
    cursors = c;
    return c->f;
}

int endmntent(FILE *f) {
    for (mnt_cursor_t **pp = &cursors; *pp; pp = &(*pp)->link) {
        if ((*pp)->f == f) {
            mnt_cursor_t *c = *pp;
            *pp = c->link;
            fclose(c->f);
            free(c);
            break;
        }
    }
    return 1;
}

// fsname, dir, type and opts one after another in buf
static char *put_field(char **at, char *end, const char *s) {
    size_t n = strlen(s) + 1;
    if (*at + n > end)
        return NULL;
    char *field = memcpy(*at, s, n);
    *at += n;
    return field;
}

struct mntent *getmntent_r(FILE *f, struct mntent *m, char *buf, int len) {
    mnt_cursor_t *c = cursor_of(f);
    user_mount_info_t info;
    if (!c || len <= 0 || _mountinfo(c->next, &info) != 0)
        return NULL;
    c->next++;
    char *at = buf, *end = buf + len;
    // linux says none for filesystems with no device behind them
    m->mnt_fsname = put_field(&at, end, info.device[0] ? info.device : "none");
    m->mnt_dir    = put_field(&at, end, info.path);
    m->mnt_type   = put_field(&at, end, info.fs);
    m->mnt_opts   = put_field(&at, end, "rw");
    if (!m->mnt_fsname || !m->mnt_dir || !m->mnt_type || !m->mnt_opts)
        return NULL;
    m->mnt_freq   = 0;
    m->mnt_passno = 0;
    return m;
}

struct mntent *getmntent(FILE *f) {
    static struct mntent m;
    static char buf[512];
    return getmntent_r(f, &m, buf, sizeof(buf));
}

int addmntent(FILE *f, const struct mntent *m) {
    (void)f; (void)m;
    return 1;
}

char *hasmntopt(const struct mntent *m, const char *opt) {
    size_t n = strlen(opt);
    for (char *p = m->mnt_opts; p && *p; ) {
        if (strncmp(p, opt, n) == 0 && (p[n] == '\0' || p[n] == ',' || p[n] == '='))
            return p;
        p = strchr(p, ',');
        if (p)
            p++;
    }
    return NULL;
}
