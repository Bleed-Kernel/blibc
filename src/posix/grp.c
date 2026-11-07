#include <errno.h>
#include <grp.h>
#include <string.h>

static char *no_members[] = { NULL };
static struct group root_group = { "root", "x", 0, no_members };
static int ent_done;

struct group *getgrgid(gid_t gid) {
    return gid == 0 ? &root_group : NULL;
}

struct group *getgrnam(const char *name) {
    return name && strcmp(name, "root") == 0 ? &root_group : NULL;
}

static int copy_group(struct group *found, struct group *grp, char *buf, size_t len, struct group **res) {
    *res = NULL;
    if (!found)
        return 0;
    size_t name = strlen(found->gr_name) + 1, pw = strlen(found->gr_passwd) + 1;
    size_t need = sizeof(char *) + name + pw;
    if (len < need)
        return ERANGE;
    char **mem = (char **)buf;
    mem[0] = NULL;
    char *p = buf + sizeof(char *);
    grp->gr_name = memcpy(p, found->gr_name, name);
    grp->gr_passwd = memcpy(p + name, found->gr_passwd, pw);
    grp->gr_gid = found->gr_gid;
    grp->gr_mem = mem;
    *res = grp;
    return 0;
}

int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t len, struct group **res) {
    return copy_group(getgrgid(gid), grp, buf, len, res);
}

int getgrnam_r(const char *name, struct group *grp, char *buf, size_t len, struct group **res) {
    return copy_group(getgrnam(name), grp, buf, len, res);
}

struct group *getgrent(void) {
    if (ent_done)
        return NULL;
    ent_done = 1;
    return &root_group;
}

void setgrent(void) { ent_done = 0; }
void endgrent(void) { ent_done = 0; }

int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups) {
    (void)user;
    int n = group == 0 ? 1 : 2;
    int fit = *ngroups;
    if (fit > 0)
        groups[0] = group;
    if (n == 2 && fit > 1)
        groups[1] = 0;
    *ngroups = n;
    return n > fit ? -1 : n;
}

int initgroups(const char *user, gid_t group) {
    (void)user; (void)group;
    return 0;
}
