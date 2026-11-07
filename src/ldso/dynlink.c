#ifdef BLIBC_SHARED
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <elf.h>
#include <link.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../thread/thread.h"

#define PAGE    4096UL
#define PAGE_UP(x)   (((x) + PAGE - 1) & ~(PAGE - 1))
#define PAGE_DOWN(x) ((x) & ~(PAGE - 1))

typedef struct dso {
    struct dso *next;
    char       *name;
    char       *path;
    uintptr_t   base;
    Elf64_Dyn  *dynamic;
    const Elf64_Phdr *phdr;
    int         phnum;

    const Elf64_Sym *syms;
    const char      *strs;
    const uint32_t  *hash;
    const uint32_t  *gnu_hash;
    const char      *runpath;

    int  tls_id;
    int  refs;
    int  relocated;
    int  relr_done;
    int  inited;
    int  global;
    int  listed;
    struct dso *local_root;

    struct dso **deps;
    int          ndeps;
} dso_t;

int __blibc_tls_add_module(const void *image, size_t filesz, size_t memsz, size_t align);
int __blibc_tls_add_dynamic(const void *image, size_t filesz, size_t memsz, size_t align);
intptr_t __blibc_tls_offset(int id);
void __blibc_tls_bootstrap(void);
void __blibc_tls_init_main(int surplus);
void __blibc_set_environ(char **envp);

static dso_t  main_dso, libc_dso;
static dso_t *head, *tail;
static const char *lib_path;
static unsigned long long adds, subs;

// a recursive lock, constructors run under it can dlopen
static volatile int dl_lock;
static int dl_owner, dl_depth;

static void lock(void) {
    int tid = thread_self()->tid;
    if (dl_owner == tid) {
        dl_depth++;
        return;
    }
    lock_take(&dl_lock);
    dl_owner = tid;
    dl_depth = 1;
}

static void unlock(void) {
    if (--dl_depth)
        return;
    dl_owner = 0;
    lock_give(&dl_lock);
}

static char dl_error_buf[256];
static int  dl_error_set;

static void set_error(const char *fmt, const char *a, const char *b) {
    snprintf(dl_error_buf, sizeof(dl_error_buf), fmt, a, b ? b : "");
    dl_error_set = 1;
}

// startup has nobody to tell, so it says so and stops
static void fatal(void) {
    write(2, "ld.so: ", 7);
    write(2, dl_error_buf, strlen(dl_error_buf));
    write(2, "\n", 1);
    _exit(127);
}

static void decode(dso_t *d) {
    for (Elf64_Dyn *dyn = d->dynamic; dyn && dyn->d_tag != DT_NULL; dyn++) {
        switch (dyn->d_tag) {
            case DT_SYMTAB:   d->syms     = (const Elf64_Sym *)(d->base + dyn->d_un.d_ptr); break;
            case DT_STRTAB:   d->strs     = (const char *)(d->base + dyn->d_un.d_ptr); break;
            case DT_HASH:     d->hash     = (const uint32_t *)(d->base + dyn->d_un.d_ptr); break;
            case DT_GNU_HASH: d->gnu_hash = (const uint32_t *)(d->base + dyn->d_un.d_ptr); break;
        }
    }
    for (Elf64_Dyn *dyn = d->dynamic; dyn && dyn->d_tag != DT_NULL; dyn++) {
        if ((dyn->d_tag == DT_RUNPATH || dyn->d_tag == DT_RPATH) && d->strs)
            d->runpath = d->strs + dyn->d_un.d_val;
        if (dyn->d_tag == DT_SONAME && d->strs && !d->name)
            d->name = (char *)d->strs + dyn->d_un.d_val;
    }
}

static uint64_t dyn_val(const dso_t *d, int64_t tag, int *found) {
    for (Elf64_Dyn *dyn = d->dynamic; dyn && dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == tag) {
            if (found)
                *found = 1;
            return dyn->d_un.d_val;
        }
    }
    if (found)
        *found = 0;
    return 0;
}

static void append(dso_t *d) {
    d->listed = 1;
    d->next = NULL;
    if (tail)
        tail->next = d;
    else
        head = d;
    tail = d;
    adds++;
}

// symbols

static uint32_t sysv_hash(const char *s) {
    uint32_t h = 0;
    while (*s) {
        h = (h << 4) + (unsigned char)*s++;
        uint32_t g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

static uint32_t gnu_hash(const char *s) {
    uint32_t h = 5381;
    while (*s)
        h = h * 33 + (unsigned char)*s++;
    return h;
}

static int defines(const Elf64_Sym *s, int plt_ok) {
    int bind = ELF64_ST_BIND(s->st_info);
    if (s->st_shndx == SHN_UNDEF &&
        !(plt_ok && s->st_value && ELF64_ST_TYPE(s->st_info) == STT_FUNC))
        return 0;
    if (bind != STB_GLOBAL && bind != STB_WEAK && bind != STB_GNU_UNIQUE)
        return 0;
    return ELF64_ST_TYPE(s->st_info) != STT_SECTION && ELF64_ST_TYPE(s->st_info) != STT_FILE;
}

static const Elf64_Sym *find_in(const dso_t *d, const char *name, uint32_t gh, uint32_t sh, int plt_ok) {
    if (!d->syms || !d->strs)
        return NULL;

    if (d->gnu_hash) {
        const uint32_t *h      = d->gnu_hash;
        uint32_t        nb     = h[0];
        uint32_t        symoff = h[1];
        uint32_t        bloom  = h[2];
        const uint32_t *bucket = h + 4 + bloom * 2;
        const uint32_t *chain  = bucket + nb;
        if (!nb)
            return NULL;
        uint32_t i = bucket[gh % nb];
        if (i < symoff)
            return NULL;
        for (;; i++) {
            uint32_t ch = chain[i - symoff];
            if ((ch | 1) == (gh | 1)) {
                const Elf64_Sym *s = &d->syms[i];
                if (defines(s, plt_ok) && !strcmp(d->strs + s->st_name, name))
                    return s;
            }
            if (ch & 1)
                return NULL;
        }
    }

    if (d->hash) {
        uint32_t        nb     = d->hash[0];
        const uint32_t *bucket = d->hash + 2;
        const uint32_t *chain  = bucket + nb;
        for (uint32_t i = bucket[sh % nb]; i; i = chain[i]) {
            const Elf64_Sym *s = &d->syms[i];
            if (defines(s, plt_ok) && !strcmp(d->strs + s->st_name, name))
                return s;
        }
    }
    return NULL;
}

// how many entries the dynamic symbol table has, the hash tables are the only thing that knows
static size_t sym_count(const dso_t *d) {
    if (d->hash)
        return d->hash[1];
    if (!d->gnu_hash)
        return 0;
    const uint32_t *h      = d->gnu_hash;
    uint32_t        nb     = h[0];
    uint32_t        symoff = h[1];
    const uint32_t *bucket = h + 4 + h[2] * 2;
    const uint32_t *chain  = bucket + nb;
    uint32_t        last   = 0;
    for (uint32_t b = 0; b < nb; b++) {
        if (bucket[b] > last)
            last = bucket[b];
    }
    if (last < symoff)
        return symoff;
    while (!(chain[last - symoff] & 1))
        last++;
    return last + 1;
}

typedef struct {
    const Elf64_Sym *sym;
    dso_t           *dso;
} def_t;

static int in_tree(const dso_t *root, const dso_t *d, int depth) {
    if (root == d)
        return 1;
    if (depth > 64)
        return 0;
    for (int i = 0; i < root->ndeps; i++) {
        if (in_tree(root->deps[i], d, depth + 1))
            return 1;
    }
    return 0;
}

static def_t lookup(const char *name, const dso_t *from, const dso_t *skip, int plt_ok) {
    uint32_t gh = gnu_hash(name), sh = sysv_hash(name);
    def_t    weak = { NULL, NULL };

    for (dso_t *d = head; d; d = d->next) {
        if (d == skip)
            continue;
        if (!d->global && !(from && from->local_root && in_tree(from->local_root, d, 0)))
            continue;
        const Elf64_Sym *s = find_in(d, name, gh, sh, plt_ok && d == &main_dso);
        if (!s)
            continue;
        def_t found = { s, d };
        if (ELF64_ST_BIND(s->st_info) != STB_WEAK)
            return found;
        // libc.so's weak c++ fallbacks lose to anyone elses, libc++.so's new/delete
        if (!weak.sym || (weak.dso == &libc_dso && d != &libc_dso))
            weak = found;
    }
    return weak;
}

// relocation

static int relocate_table(dso_t *d, const Elf64_Rela *r, size_t bytes) {
    for (size_t i = 0; i < bytes / sizeof(Elf64_Rela); i++) {
        uint32_t   type  = ELF64_R_TYPE(r[i].r_info);
        uint32_t   symi  = ELF64_R_SYM(r[i].r_info);
        uintptr_t *where = (uintptr_t *)(d->base + r[i].r_offset);
        int64_t    add   = r[i].r_addend;

        if (type == R_X86_64_NONE)
            continue;
        if (type == R_X86_64_RELATIVE) {
            *where = d->base + add;
            continue;
        }
        if (type == R_X86_64_IRELATIVE) {
            *where = ((uintptr_t (*)(void))(d->base + add))();
            continue;
        }

        def_t def = { NULL, d };
        const Elf64_Sym *s = symi ? &d->syms[symi] : NULL;
        if (s && ELF64_ST_BIND(s->st_info) == STB_LOCAL) {
            def.sym = s;
        } else if (s) {
            const char *name = d->strs + s->st_name;
            def = lookup(name, d, type == R_X86_64_COPY ? d : NULL, type != R_X86_64_JUMP_SLOT);
            if (!def.sym) {
                if (ELF64_ST_BIND(s->st_info) != STB_WEAK) {
                    set_error("%s: symbol %s not found", d->name ? d->name : "?", name);
                    return -1;
                }
                def.dso = NULL;
            }
        }

        uintptr_t S = 0;
        if (def.sym) {
            S = def.dso->base + def.sym->st_value;
            if (ELF64_ST_TYPE(def.sym->st_info) == STT_GNU_IFUNC)
                S = ((uintptr_t (*)(void))S)();
        }

        switch (type) {
            case R_X86_64_64:
                *where = S + add;
                break;
            case R_X86_64_GLOB_DAT:
            case R_X86_64_JUMP_SLOT:
                *where = S;
                break;
            case R_X86_64_COPY:
                if (def.sym)
                    memcpy(where, (void *)S, def.sym->st_size);
                break;
            case R_X86_64_DTPMOD64:
                *where = def.dso ? (uintptr_t)def.dso->tls_id : 0;
                break;
            case R_X86_64_DTPOFF64:
                *where = (def.sym ? def.sym->st_value : 0) + add;
                break;
            case R_X86_64_TPOFF64:
                *where = def.dso ? (uintptr_t)(__blibc_tls_offset(def.dso->tls_id) + (intptr_t)def.sym->st_value + add) : 0;
                break;
            default: {
                char t[16];
                snprintf(t, sizeof(t), "%u", type);
                set_error("%s: relocation type %s isnt supported", d->name ? d->name : "?", t);
                return -1;
            }
        }
    }
    return 0;
}

static void relocate_relr(dso_t *d) {
    int found;
    uintptr_t relr   = dyn_val(d, DT_RELR, &found);
    size_t    relrsz = dyn_val(d, DT_RELRSZ, NULL);
    if (!found || d->relr_done)
        return;
    d->relr_done = 1;
    const uint64_t *e   = (const uint64_t *)(d->base + relr);
    const uint64_t *end = e + relrsz / sizeof(uint64_t);
    uintptr_t *where = NULL;
    for (; e < end; e++) {
        if (!(*e & 1)) {
            where = (uintptr_t *)(d->base + *e);
            *where++ += d->base;
            continue;
        }
        uint64_t bits = *e >> 1;
        for (int i = 0; bits; i++, bits >>= 1)
            if (bits & 1)
                where[i] += d->base;
        where += 63;
    }
}

static int relocate(dso_t *d) {
    int found;
    relocate_relr(d);
    uintptr_t rela   = dyn_val(d, DT_RELA, &found);
    size_t    relasz = dyn_val(d, DT_RELASZ, NULL);
    if (found && relocate_table(d, (const Elf64_Rela *)(d->base + rela), relasz) < 0)
        return -1;
    uintptr_t jmprel = dyn_val(d, DT_JMPREL, &found);
    size_t    pltsz  = dyn_val(d, DT_PLTRELSZ, NULL);
    if (found && relocate_table(d, (const Elf64_Rela *)(d->base + jmprel), pltsz) < 0)
        return -1;
    d->relocated = 1;
    return 0;
}

// loading

static int phdr_tls(dso_t *d, int dynamic) {
    for (int i = 0; i < d->phnum; i++) {
        if (d->phdr[i].p_type != PT_TLS || !d->phdr[i].p_memsz)
            continue;
        const Elf64_Phdr *p = &d->phdr[i];
        void *image = (void *)(d->base + p->p_vaddr);
        d->tls_id = dynamic ? __blibc_tls_add_dynamic(image, p->p_filesz, p->p_memsz, p->p_align)
                            : __blibc_tls_add_module(image, p->p_filesz, p->p_memsz, p->p_align);
        if (d->tls_id < 0) {
            set_error("%s: no room left for its thread locals", d->name, NULL);
            return -1;
        }
    }
    return 0;
}

static int read_at(int fd, void *buf, size_t len, off_t off) {
    if (lseek(fd, off, SEEK_SET) != off)
        return -1;
    return read(fd, buf, len) == (ssize_t)len ? 0 : -1;
}

// maps the file's PT_LOADs into one reserved span
static dso_t *map_file(const char *path, int fd) {
    Elf64_Ehdr eh;
    if (read_at(fd, &eh, sizeof(eh), 0) < 0 || memcmp(eh.e_ident, ELFMAG, SELFMAG) ||
        eh.e_ident[4] != ELFCLASS64 || eh.e_machine != EM_X86_64 || eh.e_type != ET_DYN ||
        eh.e_phentsize != sizeof(Elf64_Phdr) || !eh.e_phnum || eh.e_phnum > 64) {
        set_error("%s: not a shared library for this machine", path, NULL);
        return NULL;
    }

    Elf64_Phdr ph[64];
    if (read_at(fd, ph, eh.e_phnum * sizeof(Elf64_Phdr), eh.e_phoff) < 0) {
        set_error("%s: cant read its program headers", path, NULL);
        return NULL;
    }

    uintptr_t lo = (uintptr_t)-1, hi = 0;
    for (int i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;
        if (PAGE_DOWN(ph[i].p_vaddr) < lo)
            lo = PAGE_DOWN(ph[i].p_vaddr);
        if (PAGE_UP(ph[i].p_vaddr + ph[i].p_memsz) > hi)
            hi = PAGE_UP(ph[i].p_vaddr + ph[i].p_memsz);
    }
    if (lo >= hi) {
        set_error("%s: nothing to load", path, NULL);
        return NULL;
    }

    char *span = mmap(NULL, hi - lo, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (span == MAP_FAILED) {
        set_error("%s: no address space for it", path, NULL);
        return NULL;
    }
    uintptr_t base = (uintptr_t)span - lo;

    for (int i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;
        int prot = ((ph[i].p_flags & PF_R) ? PROT_READ : 0) | ((ph[i].p_flags & PF_W) ? PROT_WRITE : 0) |
                   ((ph[i].p_flags & PF_X) ? PROT_EXEC : 0);
        uintptr_t start    = base + PAGE_DOWN(ph[i].p_vaddr);
        uintptr_t file_end = base + ph[i].p_vaddr + ph[i].p_filesz;
        uintptr_t mem_end  = base + ph[i].p_vaddr + ph[i].p_memsz;
        int       has_bss  = ph[i].p_memsz > ph[i].p_filesz;

        if (ph[i].p_filesz) {
            // the bss starts part way through the last file page, which then needs zeroing
            int p = prot | (has_bss ? PROT_WRITE : 0);
            if (mmap((void *)start, PAGE_UP(file_end) - start, p, MAP_PRIVATE | MAP_FIXED, fd,
                     PAGE_DOWN(ph[i].p_offset)) == MAP_FAILED) {
                set_error("%s: mapping a segment failed", path, NULL);
                return NULL;
            }
            if (has_bss && (file_end & (PAGE - 1)))
                memset((void *)file_end, 0, PAGE - (file_end & (PAGE - 1)));
        }
        uintptr_t anon = ph[i].p_filesz ? PAGE_UP(file_end) : start;
        if (has_bss && PAGE_UP(mem_end) > anon) {
            if (mmap((void *)anon, PAGE_UP(mem_end) - anon, prot, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0) == MAP_FAILED) {
                set_error("%s: mapping its bss failed", path, NULL);
                return NULL;
            }
        }
    }

    dso_t *d = calloc(1, sizeof(dso_t));
    if (!d)
        return NULL;
    d->base  = base;
    d->path  = strdup(path);
    d->phnum = eh.e_phnum;

    // the headers are in the first segment for anything ld makes, a copy if not
    Elf64_Phdr *phc = NULL;
    for (int i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type == PT_PHDR)
            d->phdr = (const Elf64_Phdr *)(base + ph[i].p_vaddr);
        if (ph[i].p_type == PT_DYNAMIC)
            d->dynamic = (Elf64_Dyn *)(base + ph[i].p_vaddr);
    }
    if (!d->phdr) {
        for (int i = 0; i < eh.e_phnum && !d->phdr; i++) {
            if (ph[i].p_type == PT_LOAD && ph[i].p_offset <= eh.e_phoff &&
                eh.e_phoff + eh.e_phnum * sizeof(Elf64_Phdr) <= ph[i].p_offset + ph[i].p_filesz)
                d->phdr = (const Elf64_Phdr *)(base + ph[i].p_vaddr + eh.e_phoff - ph[i].p_offset);
        }
    }
    if (!d->phdr && (phc = malloc(eh.e_phnum * sizeof(Elf64_Phdr)))) {
        memcpy(phc, ph, eh.e_phnum * sizeof(Elf64_Phdr));
        d->phdr = phc;
    }
    if (!d->dynamic) {
        set_error("%s: has no dynamic section", path, NULL);
        return NULL;
    }
    decode(d);
    return d;
}

static int try_open(const char *dir, size_t dlen, const char *name, char *out, size_t out_len) {
    if (dlen + 1 + strlen(name) + 1 > out_len)
        return -1;
    memcpy(out, dir, dlen);
    out[dlen] = '/';
    strcpy(out + dlen + 1, name);
    return open(out, O_RDONLY | O_CLOEXEC);
}

// a colon list of directories, $ORIGIN is the requesting module's own directory
static int search(const char *list, const dso_t *from, const char *name, char *out, size_t out_len) {
    while (list && *list) {
        const char *end = strchr(list, ':');
        size_t      len = end ? (size_t)(end - list) : strlen(list);
        char        dir[512];

        if (len >= 7 && !strncmp(list, "$ORIGIN", 7) && from && from->path) {
            const char *slash = strrchr(from->path, '/');
            size_t      olen  = slash ? (size_t)(slash - from->path) : 0;
            if (olen + len - 7 < sizeof(dir)) {
                memcpy(dir, from->path, olen);
                memcpy(dir + olen, list + 7, len - 7);
                int fd = try_open(dir, olen + len - 7, name, out, out_len);
                if (fd >= 0)
                    return fd;
            }
        } else if (len && len < sizeof(dir)) {
            int fd = try_open(list, len, name, out, out_len);
            if (fd >= 0)
                return fd;
        }
        list = end ? end + 1 : NULL;
    }
    return -1;
}

static dso_t *find_loaded(const char *name) {
    if (!strcmp(name, "libc.so") || (libc_dso.path && !strcmp(name, libc_dso.path)))
        return &libc_dso;
    for (dso_t *d = head; d; d = d->next) {
        if ((d->name && !strcmp(d->name, name)) || (d->path && !strcmp(d->path, name)))
            return d;
        const char *base = d->path ? strrchr(d->path, '/') : NULL;
        if (base && !strcmp(base + 1, name))
            return d;
    }
    return NULL;
}

static dso_t *load(const char *name, const dso_t *from, int dynamic) {
    dso_t *d = find_loaded(name);
    if (d) {
        if (!d->listed)
            append(d);
        d->refs++;
        return d;
    }

    char path[512];
    int  fd = -1;
    if (strchr(name, '/')) {
        snprintf(path, sizeof(path), "%s", name);
        fd = open(path, O_RDONLY | O_CLOEXEC);
    } else {
        fd = search(lib_path, from, name, path, sizeof(path));
        if (fd < 0 && from && from->runpath)
            fd = search(from->runpath, from, name, path, sizeof(path));
        if (fd < 0)
            fd = search("/lib:/usr/lib:/usr/local/lib", from, name, path, sizeof(path));
    }
    if (fd < 0) {
        set_error("%s: cant find it", name, NULL);
        return NULL;
    }

    d = map_file(path, fd);
    close(fd);
    if (!d)
        return NULL;
    if (!d->name || strchr(name, '/') == NULL)
        d->name = strdup(name);
    d->refs = 1;
    d->global = !dynamic;

    if (dynamic && phdr_tls(d, 1) < 0)
        return NULL;
    append(d);
    return d;
}

// the DT_NEEDED of everything from first on, breadth first
static int load_deps(dso_t *first, int dynamic) {
    for (dso_t *d = first; d; d = d->next) {
        if (d->deps || !d->dynamic)
            continue;
        int n = 0;
        for (Elf64_Dyn *dyn = d->dynamic; dyn->d_tag != DT_NULL; dyn++)
            n += dyn->d_tag == DT_NEEDED;
        d->deps = calloc(n + 1, sizeof(dso_t *));
        if (!d->deps)
            return -1;
        for (Elf64_Dyn *dyn = d->dynamic; dyn->d_tag != DT_NULL; dyn++) {
            if (dyn->d_tag != DT_NEEDED)
                continue;
            dso_t *dep = load(d->strs + dyn->d_un.d_val, d, dynamic);
            if (!dep)
                return -1;
            if (d->local_root && !dep->global && !dep->local_root)
                dep->local_root = d->local_root;
            d->deps[d->ndeps++] = dep;
        }
    }
    return 0;
}

static void call_array(uintptr_t base, const dso_t *d, int64_t tag, int64_t sztag, int reverse) {
    int       found;
    uintptr_t arr = dyn_val(d, tag, &found);
    if (!found)
        return;
    size_t n = dyn_val(d, sztag, NULL) / sizeof(uintptr_t);
    void (**fns)(void) = (void (**)(void))(base + arr);
    for (size_t i = 0; i < n; i++) {
        void (*fn)(void) = fns[reverse ? n - 1 - i : i];
        if (fn && fn != (void (*)(void))-1 && fn != (void (*)(void))0)
            fn();
    }
}

// dependencies before the ones that need them
static void init_one(dso_t *d) {
    if (d->inited)
        return;
    d->inited = 1;
    for (int i = 0; i < d->ndeps; i++)
        init_one(d->deps[i]);

    if (d == &main_dso)
        call_array(d->base, d, DT_PREINIT_ARRAY, DT_PREINIT_ARRAYSZ, 0);
    int found;
    uintptr_t init = dyn_val(d, DT_INIT, &found);
    if (found && init)
        ((void (*)(void))(d->base + init))();
    call_array(d->base, d, DT_INIT_ARRAY, DT_INIT_ARRAYSZ, 0);
}

// exit runs these, newest first, the program before what it uses
void __dl_fini(void) {
    dso_t *order[256];
    int    n = 0;
    for (dso_t *d = head; d && n < 256; d = d->next) {
        if (d->inited)
            order[n++] = d;
    }
    // later loads went in after what they need, so the list backwards is the right way round
    for (int i = n - 1; i >= 0; i--) {
        dso_t *d = order[i];
        if (d == &main_dso)
            continue;
        call_array(d->base, d, DT_FINI_ARRAY, DT_FINI_ARRAYSZ, 1);
        int found;
        uintptr_t fini = dyn_val(d, DT_FINI, &found);
        if (found && fini)
            ((void (*)(void))(d->base + fini))();
    }
    if (main_dso.inited) {
        call_array(main_dso.base, &main_dso, DT_FINI_ARRAY, DT_FINI_ARRAYSZ, 1);
        int found;
        uintptr_t fini = dyn_val(&main_dso, DT_FINI, &found);
        if (found && fini)
            ((void (*)(void))(main_dso.base + fini))();
    }
}

static const char *getenv_from(char **envp, const char *name) {
    size_t n = strlen(name);
    for (; envp && *envp; envp++) {
        if (!strncmp(*envp, name, n) && (*envp)[n] == '=')
            return *envp + n + 1;
    }
    return NULL;
}

// called once we are relocated against ourselves, gives back where the program starts
__attribute__((visibility("hidden")))
uintptr_t __dl_main(uint64_t *sp, uintptr_t base) {
    int     argc = (int)sp[0];
    char  **argv = (char **)(sp + 1);
    char  **envp = argv + argc + 1;
    char  **e    = envp;
    while (*e)
        e++;
    uint64_t *auxv = (uint64_t *)(e + 1);

    uintptr_t at_phdr = 0, at_phnum = 0, at_entry = 0, at_base = 0;
    for (uint64_t *a = auxv; a[0] != AT_NULL; a += 2) {
        if (a[0] == AT_PHDR)  at_phdr  = a[1];
        if (a[0] == AT_PHNUM) at_phnum = a[1];
        if (a[0] == AT_ENTRY) at_entry = a[1];
        if (a[0] == AT_BASE)  at_base  = a[1];
    }

    __blibc_tls_bootstrap();

    const Elf64_Ehdr *own = (const Elf64_Ehdr *)base;
    libc_dso.base    = base;
    libc_dso.name    = "libc.so";
    libc_dso.path    = "/lib/libc.so";
    libc_dso.phdr    = (const Elf64_Phdr *)(base + own->e_phoff);
    libc_dso.phnum   = own->e_phnum;
    libc_dso.global  = 1;
    libc_dso.refs    = 1;
    for (int i = 0; i < libc_dso.phnum; i++) {
        if (libc_dso.phdr[i].p_type == PT_DYNAMIC)
            libc_dso.dynamic = (Elf64_Dyn *)(base + libc_dso.phdr[i].p_vaddr);
    }
    decode(&libc_dso);
    libc_dso.name = "libc.so";

    if (!at_base || !at_phdr) {
        static const char msg[] = "libc.so is the dynamic linker, it runs programs that name it, not by itself\n";
        write(2, msg, sizeof(msg) - 1);
        _exit(1);
    }

    main_dso.phdr   = (const Elf64_Phdr *)at_phdr;
    main_dso.phnum  = (int)at_phnum;
    main_dso.global = 1;
    main_dso.refs   = 1;
    main_dso.name   = argv[0] ? argv[0] : "";
    main_dso.path   = argv[0] ? argv[0] : "";
    for (int i = 0; i < main_dso.phnum; i++) {
        if (main_dso.phdr[i].p_type == PT_PHDR)
            main_dso.base = at_phdr - main_dso.phdr[i].p_vaddr;
    }
    for (int i = 0; i < main_dso.phnum; i++) {
        if (main_dso.phdr[i].p_type == PT_DYNAMIC)
            main_dso.dynamic = (Elf64_Dyn *)(main_dso.base + main_dso.phdr[i].p_vaddr);
    }
    decode(&main_dso);
    main_dso.name = argv[0] ? argv[0] : "";

    lib_path = getenv_from(envp, "LD_LIBRARY_PATH");

    append(&main_dso);
    if (load_deps(&main_dso, 0) < 0)
        fatal();
    if (!libc_dso.listed)
        append(&libc_dso);
    if (!libc_dso.deps)
        libc_dso.deps = calloc(1, sizeof(dso_t *));

    // tls in load order, the program first so its local exec offsets come out right
    for (dso_t *d = head; d; d = d->next) {
        if (phdr_tls(d, 0) < 0)
            fatal();
    }
    __blibc_tls_init_main(1);

    // what others need first, the program last so its copies come from relocated data
    dso_t *order[256];
    int    n = 0;
    for (dso_t *d = head; d && n < 256; d = d->next)
        order[n++] = d;
    for (int i = n - 1; i >= 0; i--) {
        if (relocate(order[i]) < 0)
            fatal();
    }

    __blibc_set_environ(envp);
    init_one(&main_dso);
    for (dso_t *d = head; d; d = d->next)
        init_one(d);
    return at_entry;
}

// the dl api

static dso_t *owner_of(const void *addr) {
    uintptr_t a = (uintptr_t)addr;
    for (dso_t *d = head; d; d = d->next) {
        for (int i = 0; i < d->phnum; i++) {
            const Elf64_Phdr *p = &d->phdr[i];
            if (p->p_type == PT_LOAD && a >= d->base + p->p_vaddr && a < d->base + p->p_vaddr + p->p_memsz)
                return d;
        }
    }
    return NULL;
}

void *dlopen(const char *file, int mode) {
    lock();
    if (!file) {
        unlock();
        return &main_dso;
    }

    dso_t *d = find_loaded(file);
    if (d || (mode & RTLD_NOLOAD)) {
        if (d) {
            d->refs++;
            if (mode & RTLD_GLOBAL)
                d->global = 1;
        } else {
            set_error("%s: not loaded", file, NULL);
        }
        unlock();
        return d;
    }

    dso_t *was_tail = tail;
    d = load(file, owner_of(__builtin_return_address(0)), 1);
    if (!d) {
        unlock();
        return NULL;
    }
    dso_t *first = was_tail ? was_tail->next : head;
    for (dso_t *x = first; x; x = x->next) {
        x->global = (mode & RTLD_GLOBAL) != 0;
        if (!x->global)
            x->local_root = d;
    }
    if (load_deps(first, 1) < 0) {
        unlock();
        return NULL;
    }
    for (dso_t *x = first; x; x = x->next) {
        x->global = (mode & RTLD_GLOBAL) != 0;
        if (!x->global && !x->local_root)
            x->local_root = d;
    }

    dso_t *order[256];
    int    n = 0;
    for (dso_t *x = first; x && n < 256; x = x->next)
        order[n++] = x;
    for (int i = n - 1; i >= 0; i--) {
        if (!order[i]->relocated && relocate(order[i]) < 0) {
            unlock();
            return NULL;
        }
    }
    unlock();

    // constructors can dlopen too, so not with the lock
    init_one(d);
    for (dso_t *x = first; x; x = x->next)
        init_one(x);
    return d;
}

static void *sym_addr(const Elf64_Sym *s, const dso_t *d) {
    uintptr_t a = d->base + s->st_value;
    if (ELF64_ST_TYPE(s->st_info) == STT_TLS)
        return (char *)thread_self() + __blibc_tls_offset(d->tls_id) + s->st_value;
    if (ELF64_ST_TYPE(s->st_info) == STT_GNU_IFUNC)
        a = ((uintptr_t (*)(void))a)();
    return (void *)a;
}

static const Elf64_Sym *find_tree(dso_t *root, const char *name, dso_t **where, int depth) {
    uint32_t gh = gnu_hash(name), sh = sysv_hash(name);
    const Elf64_Sym *s = find_in(root, name, gh, sh, root == &main_dso);
    if (s) {
        *where = root;
        return s;
    }
    if (depth > 64)
        return NULL;
    for (int i = 0; i < root->ndeps; i++) {
        s = find_tree(root->deps[i], name, where, depth + 1);
        if (s)
            return s;
    }
    return NULL;
}

void *dlsym(void *handle, const char *name) {
    lock();
    const Elf64_Sym *s   = NULL;
    dso_t           *def = NULL;

    if (handle == RTLD_DEFAULT || handle == &main_dso) {
        def_t r = lookup(name, owner_of(__builtin_return_address(0)), NULL, 1);
        s   = r.sym;
        def = r.dso;
    } else if (handle == RTLD_NEXT) {
        dso_t   *caller = owner_of(__builtin_return_address(0));
        uint32_t gh = gnu_hash(name), sh = sysv_hash(name);
        for (dso_t *d = caller ? caller->next : NULL; d && !s; d = d->next) {
            if ((s = find_in(d, name, gh, sh, d == &main_dso)))
                def = d;
        }
    } else {
        s = find_tree((dso_t *)handle, name, &def, 0);
    }

    if (!s) {
        set_error("symbol %s not found", name, NULL);
        unlock();
        return NULL;
    }
    void *a = sym_addr(s, def);
    unlock();
    return a;
}

// nothing is ever unmapped
int dlclose(void *handle) {
    lock();
    dso_t *d = handle;
    if (d && d->refs > 0)
        d->refs--;
    unlock();
    return 0;
}

char *dlerror(void) {
    if (!dl_error_set)
        return NULL;
    dl_error_set = 0;
    return dl_error_buf;
}

int dladdr(const void *addr, Dl_info *info) {
    lock();
    dso_t *d = owner_of(addr);
    if (!d) {
        unlock();
        return 0;
    }
    info->dli_fname = d->path;
    info->dli_fbase = (void *)d->base;
    info->dli_sname = NULL;
    info->dli_saddr = NULL;

    uintptr_t a    = (uintptr_t)addr;
    uintptr_t best = 0;
    size_t    n    = sym_count(d);
    for (size_t i = 1; i < n; i++) {
        const Elf64_Sym *s = &d->syms[i];
        if (!defines(s, 0) || ELF64_ST_TYPE(s->st_info) == STT_TLS)
            continue;
        uintptr_t v = d->base + s->st_value;
        if (v <= a && v >= best && (a < v + s->st_size || !s->st_size)) {
            best = v;
            info->dli_sname = d->strs + s->st_name;
            info->dli_saddr = (void *)v;
        }
    }
    unlock();
    return 1;
}

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *info, size_t size, void *data), void *data) {
    int r = 0;
    for (dso_t *d = head; d && !r; d = d->next) {
        struct dl_phdr_info info;
        memset(&info, 0, sizeof(info));
        info.dlpi_addr      = d->base;
        info.dlpi_name      = d == &main_dso ? "" : d->path;
        info.dlpi_phdr      = d->phdr;
        info.dlpi_phnum     = (Elf64_Half)d->phnum;
        info.dlpi_adds      = adds;
        info.dlpi_subs      = subs;
        info.dlpi_tls_modid = d->tls_id > 0 ? (size_t)d->tls_id : 0;
        info.dlpi_tls_data  = d->tls_id > 0 ? (char *)thread_self() + __blibc_tls_offset(d->tls_id) : NULL;
        r = callback(&info, sizeof(info), data);
    }
    return r;
}
#endif
