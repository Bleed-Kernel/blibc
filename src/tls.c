#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <bleed/syscalls.h>

#define PT_TLS      7
#define ARCH_SET_FS 0x1002

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type, e_machine;
    uint32_t e_version;
    uint64_t e_entry, e_phoff, e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx;
} tls_ehdr_t;

typedef struct {
    uint32_t p_type, p_flags;
    uint64_t p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align;
} tls_phdr_t;

typedef struct {
    void *self;
    void *dtv;
    void *pad[3];
    uintptr_t stack_guard;
    uintptr_t spare[2];
} tls_tcb_t;

extern uintptr_t __stack_chk_guard;

extern const char __ehdr_start[] __attribute__((weak));

static tls_tcb_t main_tcb;
void __blibc_init_tls(void) {
    tls_tcb_t *tcb = &main_tcb;
    const tls_phdr_t *tls = NULL;

    if (__ehdr_start) {
        const tls_ehdr_t *eh = (const tls_ehdr_t *)__ehdr_start;
        const tls_phdr_t *ph = (const tls_phdr_t *)(__ehdr_start + eh->e_phoff);
        for (int i = 0; i < eh->e_phnum; i++) {
            if (ph[i].p_type == PT_TLS)
                tls = &ph[i];
        }
    }

    if (tls && tls->p_memsz) {
        size_t align = tls->p_align > 16 ? tls->p_align : 16;
        size_t size = (tls->p_memsz + align - 1) & ~(align - 1);

        char *mem = malloc(size + sizeof(tls_tcb_t) + align);
        if (mem) {
            char *block = (char *)(((uintptr_t)mem + align - 1) & ~(uintptr_t)(align - 1));
            memcpy(block, (const void *)tls->p_vaddr, tls->p_filesz);
            memset(block + tls->p_filesz, 0, size - tls->p_filesz);
            tcb = (tls_tcb_t *)(block + size);
            memset(tcb, 0, sizeof(*tcb));
        }
    }

    tcb->self = tcb;
    tcb->stack_guard = __stack_chk_guard;
    _arch_prctl(ARCH_SET_FS, (uint64_t)tcb);
}
