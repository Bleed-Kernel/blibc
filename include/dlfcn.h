#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY       0x0001
#define RTLD_NOW        0x0002
#define RTLD_NOLOAD     0x0004
#define RTLD_DEEPBIND   0x0008
#define RTLD_GLOBAL     0x0100
#define RTLD_LOCAL      0
#define RTLD_NODELETE   0x1000

#define RTLD_DEFAULT    ((void *)0)
#define RTLD_NEXT       ((void *)-1)

typedef struct {
    const char *dli_fname;
    void       *dli_fbase;
    const char *dli_sname;
    void       *dli_saddr;
} Dl_info;

// no dynamic loading yet, every program is one static image
void *dlopen(const char *file, int mode);
void *dlsym(void *handle, const char *name);
int   dlclose(void *handle);
char *dlerror(void);
int   dladdr(const void *addr, Dl_info *info);

#ifdef __cplusplus
}
#endif
