#ifdef BLIBC_SHARED
// nothing here is relocated yet
#pragma GCC visibility push(hidden)
#include <stdint.h>
#include <stddef.h>
#include <elf.h>

extern const char __ehdr_start[];
extern Elf64_Dyn  _DYNAMIC[];

uintptr_t __dl_main(uint64_t *sp, uintptr_t base);

static inline void self_apply(uintptr_t base, const Elf64_Rela *r, size_t bytes, const Elf64_Sym *syms) {
    for (size_t i = 0; i < bytes / sizeof(Elf64_Rela); i++) {
        uintptr_t *where = (uintptr_t *)(base + r[i].r_offset);
        uint32_t   type  = ELF64_R_TYPE(r[i].r_info);
        const Elf64_Sym *s = &syms[ELF64_R_SYM(r[i].r_info)];

        if (type == R_X86_64_RELATIVE) {
            *where = base + r[i].r_addend;
        } else if (type == R_X86_64_GLOB_DAT || type == R_X86_64_JUMP_SLOT) {
            if (s->st_shndx != SHN_UNDEF)
                *where = base + s->st_value;
        } else if (type == R_X86_64_64) {
            if (s->st_shndx != SHN_UNDEF)
                *where = base + s->st_value + r[i].r_addend;
        }
    }
}

uintptr_t __dl_start(uint64_t *sp) {
    uintptr_t base = (uintptr_t)__ehdr_start;
    const Elf64_Rela *rela = 0, *jmprel = 0;
    const Elf64_Sym  *syms = 0;
    size_t relasz = 0, pltsz = 0;

    for (Elf64_Dyn *d = _DYNAMIC; d->d_tag != DT_NULL; d++) {
        if (d->d_tag == DT_RELA)
            rela = (const Elf64_Rela *)(base + d->d_un.d_ptr);
        else if (d->d_tag == DT_RELASZ)
            relasz = d->d_un.d_val;
        else if (d->d_tag == DT_JMPREL)
            jmprel = (const Elf64_Rela *)(base + d->d_un.d_ptr);
        else if (d->d_tag == DT_PLTRELSZ)
            pltsz = d->d_un.d_val;
        else if (d->d_tag == DT_SYMTAB)
            syms = (const Elf64_Sym *)(base + d->d_un.d_ptr);
    }

    if (rela)
        self_apply(base, rela, relasz, syms);
    if (jmprel)
        self_apply(base, jmprel, pltsz, syms);
    return __dl_main(sp, base);
}
#endif
