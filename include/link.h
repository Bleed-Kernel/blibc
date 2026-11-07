#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <elf.h>

#define ElfW(type) Elf64_##type

struct dl_phdr_info {
    ElfW(Addr)        dlpi_addr;
    const char       *dlpi_name;
    const ElfW(Phdr) *dlpi_phdr;
    ElfW(Half)        dlpi_phnum;
    unsigned long long dlpi_adds;
    unsigned long long dlpi_subs;
    size_t            dlpi_tls_modid;
    void             *dlpi_tls_data;
};

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data);

#ifdef __cplusplus
}
#endif
