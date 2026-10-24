#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

// enough itanium abi rtti for dynamic_cast. g++ points every class's typeinfo at one of the three

typedef struct type_info {
    const void *vptr;
    const char *name;
} type_info_t;

typedef struct {
    type_info_t        info;
    const type_info_t *base;
} si_class_info_t;

#define BASE_VIRTUAL    0x1
#define BASE_PUBLIC     0x2
#define BASE_OFFSET_SHIFT 8

typedef struct {
    const type_info_t *base;
    long               offset_flags;
} base_info_t;

typedef struct {
    type_info_t  info;
    unsigned int flags;
    unsigned int base_count;
    base_info_t  bases[];
} vmi_class_info_t;

typedef struct {
    ptrdiff_t   offset_to_top;
    const void *type;
    const void *slots[8];
} info_vtable_t;

__attribute__((weak)) const info_vtable_t class_info_vtable     __asm__("_ZTVN10__cxxabiv117__class_type_infoE")     = {0};
__attribute__((weak)) const info_vtable_t si_class_info_vtable  __asm__("_ZTVN10__cxxabiv120__si_class_type_infoE")  = {0};
__attribute__((weak)) const info_vtable_t vmi_class_info_vtable __asm__("_ZTVN10__cxxabiv121__vmi_class_type_infoE") = {0};

// a typeinfo's vptr points past the offset and typeinfo words
#define IS_KIND(t, vt) ((t)->vptr == (const void *)&(vt).slots[0])

static int same_type(const type_info_t *a, const type_info_t *b) {
    if (a == b)
        return 1;
    // a '*' name is local to its file and only ever matches by address
    if (a->name[0] == '*' || b->name[0] == '*')
        return 0;
    return strcmp(a->name, b->name) == 0;
}

typedef struct {
    const type_info_t *dst;
    const type_info_t *src;
    const void        *src_ptr;
    const void        *found[8];
    int                found_count;
    const void        *containing;  // the dst subobject the src pointer sits in
    int                src_public;  // src is a public base of the whole object
} search_t;

static void add_found(search_t *s, const void *p) {
    for (int i = 0; i < s->found_count; i++)
        if (s->found[i] == p)
            return;
    if (s->found_count < (int)(sizeof(s->found) / sizeof(s->found[0])))
        s->found[s->found_count++] = p;
}

// public_path is public all the way from the whole object, dst_public only from the nearest dst
static void walk(search_t *s, const type_info_t *type, const char *obj, int public_path,
                 const void *inside_dst, int dst_public) {
    if (same_type(type, s->dst)) {
        if (public_path)
            add_found(s, obj);
        inside_dst = obj;
        dst_public = 1;
    }

    if (obj == s->src_ptr && same_type(type, s->src)) {
        if (inside_dst && dst_public)
            s->containing = inside_dst;
        if (public_path)
            s->src_public = 1;
    }

    if (IS_KIND(type, si_class_info_vtable)) {
        walk(s, ((const si_class_info_t *)type)->base, obj, public_path, inside_dst, dst_public);
        return;
    }
    if (!IS_KIND(type, vmi_class_info_vtable))
        return;

    const vmi_class_info_t *vmi = (const vmi_class_info_t *)type;
    for (unsigned i = 0; i < vmi->base_count; i++) {
        long of = vmi->bases[i].offset_flags;
        long offset = of >> BASE_OFFSET_SHIFT;
        int pub = (of & BASE_PUBLIC) != 0;

        // a virtual base's real offset is kept in the vtable, at a negative index this says
        if (of & BASE_VIRTUAL) {
            const char *vptr = *(const char *const *)obj;
            offset = *(const ptrdiff_t *)(vptr + offset);
        }
        walk(s, vmi->bases[i].base, obj + offset, public_path && pub, inside_dst, dst_public && pub);
    }
}

void *__dynamic_cast(const void *src_ptr, const type_info_t *src_type, const type_info_t *dst_type, ptrdiff_t hint) {
    (void)hint;
    if (!src_ptr)
        return NULL;

    const void *const *vptr = *(const void *const *const *)src_ptr;
    ptrdiff_t to_top = ((const ptrdiff_t *)vptr)[-2];
    const type_info_t *whole_type = vptr[-1];
    const char *whole = (const char *)src_ptr + to_top;

    // built without rtti, nothing to go on
    if (!whole_type)
        return NULL;

    search_t s = { .dst = dst_type, .src = src_type, .src_ptr = src_ptr };
    walk(&s, whole_type, whole, 1, NULL, 0);

    // a downcast picks the dst the pointer is actually in, a cross cast needs src public in
    // the whole object and just the one dst
    if (s.containing)
        return (void *)s.containing;
    if (s.src_public && s.found_count == 1)
        return (void *)s.found[0];
    return NULL;
}

// dynamic_cast to a reference that fails, and typeid of a null pointer, would throw
void __cxa_bad_cast(void) {
    fprintf(stderr, "bad dynamic_cast\n");
    abort();
}

void __cxa_bad_typeid(void) {
    fprintf(stderr, "typeid of a null pointer\n");
    abort();
}
