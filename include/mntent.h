#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOUNTED      "/etc/mtab"
#define _PATH_MOUNTED MOUNTED

#define MNTTYPE_IGNORE "ignore"
#define MNTTYPE_NFS    "nfs"
#define MNTTYPE_SWAP   "swap"
#define MNTOPT_DEFAULTS "defaults"
#define MNTOPT_RO      "ro"
#define MNTOPT_RW      "rw"
#define MNTOPT_SUID    "suid"
#define MNTOPT_NOSUID  "nosuid"
#define MNTOPT_NOAUTO  "noauto"

struct mntent {
    char *mnt_fsname;
    char *mnt_dir;
    char *mnt_type;
    char *mnt_opts;
    int   mnt_freq;
    int   mnt_passno;
};

// the mounts come from the kernels mount list, the path is ignored
FILE *setmntent(const char *path, const char *mode);
int endmntent(FILE *f);
struct mntent *getmntent(FILE *f);
struct mntent *getmntent_r(FILE *f, struct mntent *m, char *buf, int len);
int addmntent(FILE *f, const struct mntent *m);
char *hasmntopt(const struct mntent *m, const char *opt);

#ifdef __cplusplus
}
#endif
