#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// c++ operator new and delete, written in c under their mangled names so blibc stays a c build.
// no exceptions on bleed so a failed new aborts instead of throwing bad_alloc. all weak so a
// program can still replace them

#define WEAK    __attribute__((weak))

typedef struct { char unused; } nothrow_t;

WEAK const nothrow_t cxx_nothrow __asm__("_ZSt7nothrow") = {0};

static void *must_alloc(size_t size) {
    void *p = malloc(size ? size : 1);
    if (!p) {
        fprintf(stderr, "operator new: out of memory (%lu bytes)\n", (unsigned long)size);
        abort();
    }
    return p;
}

// malloc is 16 aligned already, past that the real pointer gets stashed just before the block
static void *alloc_aligned(size_t size, size_t align) {
    if (align <= 16)
        return malloc(size ? size : 1);

    char *raw = malloc(size + align + sizeof(void *));
    if (!raw)
        return NULL;

    uintptr_t p = ((uintptr_t)raw + sizeof(void *) + align - 1) & ~(uintptr_t)(align - 1);
    ((void **)p)[-1] = raw;
    return (void *)p;
}

static void free_aligned(void *p, size_t align) {
    if (!p)
        return;
    if (align <= 16)
        free(p);
    else
        free(((void **)p)[-1]);
}

static void *must_alloc_aligned(size_t size, size_t align) {
    void *p = alloc_aligned(size, align);
    if (!p) {
        fprintf(stderr, "operator new: out of memory (%lu bytes, %lu aligned)\n",
                (unsigned long)size, (unsigned long)align);
        abort();
    }
    return p;
}

// new
WEAK void *cxx_new(size_t size) __asm__("_Znwm");
WEAK void *cxx_new_array(size_t size) __asm__("_Znam");
WEAK void *cxx_new_nothrow(size_t size, const nothrow_t *) __asm__("_ZnwmRKSt9nothrow_t");
WEAK void *cxx_new_array_nothrow(size_t size, const nothrow_t *) __asm__("_ZnamRKSt9nothrow_t");
WEAK void *cxx_new_aligned(size_t size, size_t align) __asm__("_ZnwmSt11align_val_t");
WEAK void *cxx_new_array_aligned(size_t size, size_t align) __asm__("_ZnamSt11align_val_t");
WEAK void *cxx_new_aligned_nothrow(size_t size, size_t align, const nothrow_t *) __asm__("_ZnwmSt11align_val_tRKSt9nothrow_t");
WEAK void *cxx_new_array_aligned_nothrow(size_t size, size_t align, const nothrow_t *) __asm__("_ZnamSt11align_val_tRKSt9nothrow_t");

void *cxx_new(size_t size)                         { return must_alloc(size); }
void *cxx_new_array(size_t size)                   { return must_alloc(size); }
void *cxx_new_nothrow(size_t size, const nothrow_t *nt)       { (void)nt; return malloc(size ? size : 1); }
void *cxx_new_array_nothrow(size_t size, const nothrow_t *nt) { (void)nt; return malloc(size ? size : 1); }
void *cxx_new_aligned(size_t size, size_t align)       { return must_alloc_aligned(size, align); }
void *cxx_new_array_aligned(size_t size, size_t align) { return must_alloc_aligned(size, align); }
void *cxx_new_aligned_nothrow(size_t size, size_t align, const nothrow_t *nt)       { (void)nt; return alloc_aligned(size, align); }
void *cxx_new_array_aligned_nothrow(size_t size, size_t align, const nothrow_t *nt) { (void)nt; return alloc_aligned(size, align); }

// delete
WEAK void cxx_delete(void *p) __asm__("_ZdlPv");
WEAK void cxx_delete_array(void *p) __asm__("_ZdaPv");
WEAK void cxx_delete_sized(void *p, size_t size) __asm__("_ZdlPvm");
WEAK void cxx_delete_array_sized(void *p, size_t size) __asm__("_ZdaPvm");
WEAK void cxx_delete_nothrow(void *p, const nothrow_t *) __asm__("_ZdlPvRKSt9nothrow_t");
WEAK void cxx_delete_array_nothrow(void *p, const nothrow_t *) __asm__("_ZdaPvRKSt9nothrow_t");
WEAK void cxx_delete_aligned(void *p, size_t align) __asm__("_ZdlPvSt11align_val_t");
WEAK void cxx_delete_array_aligned(void *p, size_t align) __asm__("_ZdaPvSt11align_val_t");
WEAK void cxx_delete_sized_aligned(void *p, size_t size, size_t align) __asm__("_ZdlPvmSt11align_val_t");
WEAK void cxx_delete_array_sized_aligned(void *p, size_t size, size_t align) __asm__("_ZdaPvmSt11align_val_t");
WEAK void cxx_delete_aligned_nothrow(void *p, size_t align, const nothrow_t *) __asm__("_ZdlPvSt11align_val_tRKSt9nothrow_t");
WEAK void cxx_delete_array_aligned_nothrow(void *p, size_t align, const nothrow_t *) __asm__("_ZdaPvSt11align_val_tRKSt9nothrow_t");

void cxx_delete(void *p)                                { free(p); }
void cxx_delete_array(void *p)                          { free(p); }
void cxx_delete_sized(void *p, size_t size)             { (void)size; free(p); }
void cxx_delete_array_sized(void *p, size_t size)       { (void)size; free(p); }
void cxx_delete_nothrow(void *p, const nothrow_t *nt)       { (void)nt; free(p); }
void cxx_delete_array_nothrow(void *p, const nothrow_t *nt) { (void)nt; free(p); }
void cxx_delete_aligned(void *p, size_t align)          { free_aligned(p, align); }
void cxx_delete_array_aligned(void *p, size_t align)    { free_aligned(p, align); }
void cxx_delete_sized_aligned(void *p, size_t size, size_t align)       { (void)size; free_aligned(p, align); }
void cxx_delete_array_sized_aligned(void *p, size_t size, size_t align) { (void)size; free_aligned(p, align); }
void cxx_delete_aligned_nothrow(void *p, size_t align, const nothrow_t *nt)       { (void)nt; free_aligned(p, align); }
void cxx_delete_array_aligned_nothrow(void *p, size_t align, const nothrow_t *nt) { (void)nt; free_aligned(p, align); }
