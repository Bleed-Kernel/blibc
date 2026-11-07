#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <bleed/syscalls.h>
#include "thread/thread.h"

#define PT_TLS      7
#define ARCH_SET_FS 0x1002
#define PAGE_SIZE   4096
#define TLS_MODS    64
#define TLS_SURPLUS (16 * 1024)    // room for the tls of libraries dlopen'd later
#define TLS_DYN_ALIGN 64

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

extern uintptr_t __stack_chk_guard;

extern const char __ehdr_start[] __attribute__((weak));

// every module with a PT_TLS, its block sits offset bytes below the thread pointer
typedef struct {
    const void *image;
    size_t      filesz;
    size_t      memsz;
    intptr_t    offset;
} tls_mod_t;

static tls_mod_t mods[TLS_MODS];
static int       nmods;
static size_t    tls_static;
static size_t    tls_used;
static size_t    tls_align = 16;
static size_t    tls_surplus;
static volatile int tls_lock;

static struct __pthread main_thread;
static struct __pthread *threads;

size_t __blibc_tls_size(void) {
    return tls_static + tls_surplus + tls_align + sizeof(struct __pthread);
}

static void fill_module(uintptr_t tp, const tls_mod_t *m) {
    char *block = (char *)(tp + m->offset);
    if (m->filesz)
        memcpy(block, m->image, m->filesz);
    memset(block + m->filesz, 0, m->memsz - m->filesz);
}

struct __pthread *__blibc_tls_setup(void *mem, size_t size) {
    uintptr_t top = (uintptr_t)mem + size - sizeof(struct __pthread);
    uintptr_t tp  = top & ~(uintptr_t)(tls_align - 1);

    memset((void *)(tp - tls_static - tls_surplus), 0, tls_static + tls_surplus);
    for (int i = 0; i < nmods; i++)
        fill_module(tp, &mods[i]);

    struct __pthread *t = (struct __pthread *)tp;
    memset(t, 0, sizeof(*t));
    t->self        = t;
    t->stack_guard = __stack_chk_guard;
    t->errno_ptr   = &t->errno_val;
    return t;
}

// threads go on the list under the same lock a dlopen fills them in under
void __blibc_tls_thread_add(struct __pthread *t) {
    lock_take(&tls_lock);
    t->next_thread = threads;
    t->prev_thread = NULL;
    if (threads)
        threads->prev_thread = t;
    threads = t;
    lock_give(&tls_lock);
}

void __blibc_tls_thread_remove(struct __pthread *t) {
    lock_take(&tls_lock);
    if (t->prev_thread)
        t->prev_thread->next_thread = t->next_thread;
    else if (threads == t)
        threads = t->next_thread;
    if (t->next_thread)
        t->next_thread->prev_thread = t->prev_thread;
    t->next_thread = t->prev_thread = NULL;
    lock_give(&tls_lock);
}

void __blibc_tls_fork_child(struct __pthread *self) {
    tls_lock = 0;
    threads  = self;
    self->next_thread = self->prev_thread = NULL;
}

// a startup module, before the first real block is made. gives back its 1 based id
int __blibc_tls_add_module(const void *image, size_t filesz, size_t memsz, size_t align) {
    if (nmods == TLS_MODS)
        return -1;
    if (align < 1)
        align = 1;
    size_t end = (tls_static + memsz + align - 1) & ~(align - 1);
    mods[nmods] = (tls_mod_t){ image, filesz, memsz, -(intptr_t)end };
    tls_static = end;
    tls_used   = end;
    if (align > tls_align)
        tls_align = align;
    return ++nmods;
}

// a dlopen'd module goes in the surplus
int __blibc_tls_add_dynamic(const void *image, size_t filesz, size_t memsz, size_t align) {
    lock_take(&tls_lock);
    if (align < 1)
        align = 1;
    size_t end = (tls_used + memsz + align - 1) & ~(align - 1);
    if (nmods == TLS_MODS || align > tls_align || end > tls_static + tls_surplus) {
        lock_give(&tls_lock);
        return -1;
    }
    mods[nmods] = (tls_mod_t){ image, filesz, memsz, -(intptr_t)end };
    tls_used = end;
    for (struct __pthread *t = threads; t; t = t->next_thread)
        fill_module((uintptr_t)t, &mods[nmods]);
    int id = ++nmods;
    lock_give(&tls_lock);
    return id;
}

intptr_t __blibc_tls_offset(int id) {
    return id > 0 && id <= nmods ? mods[id - 1].offset : 0;
}

// general dynamic tls
typedef struct {
    unsigned long module;
    unsigned long offset;
} tls_index_t;

void *__tls_get_addr(tls_index_t *ti) {
    return (char *)thread_self() + mods[ti->module - 1].offset + ti->offset;
}

// errno and the stack guard go through %fs
void __blibc_tls_bootstrap(void) {
    main_thread.self        = &main_thread;
    main_thread.stack_guard = __stack_chk_guard;
    main_thread.errno_ptr   = &__errno_main;
    _arch_prctl(ARCH_SET_FS, (uint64_t)&main_thread);
}

void __blibc_tls_init_main(int surplus) {
    struct __pthread *self = &main_thread;
    tls_surplus = surplus ? TLS_SURPLUS : 0;
    // a dlopen'd module cant move tp, so leave room for the alignments libraries ask for (libxul is 32)
    if (tls_surplus && tls_align < TLS_DYN_ALIGN)
        tls_align = TLS_DYN_ALIGN;

    if (tls_static || tls_surplus) {
        size_t size = (__blibc_tls_size() + PAGE_SIZE - 1) & ~(size_t)(PAGE_SIZE - 1);
        void *mem = _mmap(size / PAGE_SIZE);
        if (mem && (long)mem > 0) {
            self = __blibc_tls_setup(mem, size);
            self->errno_ptr = &__errno_main;
            _arch_prctl(ARCH_SET_FS, (uint64_t)self);
        }
    }

    // the main thread runs off the kernels 8M stack
    self->stack_size = 8 * 1024 * 1024;
    self->stack_low  = (void *)(0x00007ffffffff000ULL - self->stack_size);
    self->tid = (int)thread_syscall6(SYS_SET_TID_ADDRESS, (long)&self->tid, 0, 0, 0, 0, 0);
    __blibc_tls_thread_add(self);
}

// a static program, its one PT_TLS found through its own headers
void __blibc_init_tls(void) {
    const tls_phdr_t *tls = NULL;

    __blibc_tls_bootstrap();

    if (__ehdr_start) {
        const tls_ehdr_t *eh = (const tls_ehdr_t *)__ehdr_start;
        const tls_phdr_t *ph = (const tls_phdr_t *)(__ehdr_start + eh->e_phoff);
        for (int i = 0; i < eh->e_phnum; i++) {
            if (ph[i].p_type == PT_TLS)
                tls = &ph[i];
        }
    }

    if (tls && tls->p_memsz)
        __blibc_tls_add_module((const void *)tls->p_vaddr, tls->p_filesz, tls->p_memsz, tls->p_align);
    __blibc_tls_init_main(0);
}
