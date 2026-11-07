#pragma once

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct group {
    char  *gr_name;
    char  *gr_passwd;
    gid_t  gr_gid;
    char **gr_mem;
};

// one group, root (gid 0) like pwd.h has the one user
struct group *getgrgid(gid_t gid);
struct group *getgrnam(const char *name);
int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t len, struct group **res);
int getgrnam_r(const char *name, struct group *grp, char *buf, size_t len, struct group **res);
struct group *getgrent(void);
void setgrent(void);
void endgrent(void);
int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups);
int initgroups(const char *user, gid_t group);
int setgroups(size_t size, const gid_t *list);

#ifdef __cplusplus
}
#endif
