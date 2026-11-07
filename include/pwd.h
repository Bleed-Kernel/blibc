#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

struct passwd {
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

// Bleed has no user accounts, both of these describe root with HOME and SHELL from the environment
struct passwd *getpwuid(uid_t uid);
struct passwd *getpwnam(const char *name);
int getpwuid_r(uid_t uid, struct passwd *pw, char *buf, size_t len, struct passwd **res);
int getpwnam_r(const char *name, struct passwd *pw, char *buf, size_t len, struct passwd **res);

#ifdef __cplusplus
}
#endif
