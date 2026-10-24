#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10
#define DT_SOCK    12

struct dirent {
    ino_t d_ino;
    unsigned char d_type;
    char d_name[256];
};
typedef struct dirent dirent;

typedef struct {
    int fd;
    unsigned long index;
    struct dirent entry;
} DIR;

DIR *opendir(const char *name);
DIR *fdopendir(int fd);
int dirfd(DIR *dirp);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);

#ifdef __cplusplus
}
#endif
