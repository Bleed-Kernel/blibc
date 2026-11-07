#ifndef BLIBC_SHARED

#include <link.h>
#include <dlfcn.h>
#include <string.h>

extern const char __ehdr_start[] __attribute__((weak));

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data) {
    // tcc doesnt make __ehdr_start
    if (!__ehdr_start)
        return 0;

    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)__ehdr_start;
    struct dl_phdr_info info;
    memset(&info, 0, sizeof(info));
    info.dlpi_addr  = 0;
    info.dlpi_name  = "";
    info.dlpi_phdr  = (const Elf64_Phdr *)(__ehdr_start + eh->e_phoff);
    info.dlpi_phnum = eh->e_phnum;
    info.dlpi_adds  = 1;
    return callback(&info, sizeof(info), data);
}

static const char *dl_error;

void *dlopen(const char *file, int mode) {
    (void)file;
    (void)mode;
    dl_error = "dynamic loading isnt supported";
    return NULL;
}

void *dlsym(void *handle, const char *name) {
    (void)handle;
    (void)name;
    dl_error = "symbol not found";
    return NULL;
}

int dlclose(void *handle) {
    (void)handle;
    return 0;
}

char *dlerror(void) {
    const char *e = dl_error;
    dl_error = NULL;
    return (char *)e;
}

int dladdr(const void *addr, Dl_info *info) {
    (void)addr;
    (void)info;
    return 0;
}
#endif
