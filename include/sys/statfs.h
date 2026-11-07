#pragma once

#include <sys/types.h>
#include <sys/statvfs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int __val[2]; } fsid_t;

struct statfs {
    unsigned long f_type, f_bsize;
    fsblkcnt_t f_blocks, f_bfree, f_bavail;
    fsfilcnt_t f_files, f_ffree;
    fsid_t f_fsid;
    unsigned long f_namelen, f_frsize, f_flags, f_spare[4];
};
#define statfs64 statfs
#define fstatfs64 fstatfs

int statfs(const char *path, struct statfs *buf);
int fstatfs(int fd, struct statfs *buf);

#ifdef __cplusplus
}
#endif
