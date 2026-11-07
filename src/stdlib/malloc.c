#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <bleed/syscalls.h>
#include "../thread/thread.h"

#define PAGE_SIZE       4096
#define ARENA_SIZE      (256 * 1024)
#define MMAP_THRESHOLD  (128 * 1024)

#define INUSE       1
#define PREV_INUSE  2
#define MMAPPED     4
#define FLAGS       7

#define HDR         16
#define MIN_CHUNK   32

#define SMALL_BINS  64
#define NBINS       256

typedef struct chunk {
    size_t prev_size;
    size_t size;
    struct chunk *next;
    struct chunk *prev;
} chunk_t;

_Static_assert(HDR % 16 == 0, "malloc header has to keep pointers 16 aligned");

volatile int __malloc_lock;

static chunk_t *bins[NBINS];
static uint64_t bin_map[NBINS / 64];

static inline size_t chunk_size(const chunk_t *c) {
    return c->size & ~(size_t)FLAGS;
}

static inline chunk_t *chunk_at(const chunk_t *c, size_t off) {
    return (chunk_t *)((uintptr_t)c + off);
}

static inline void *payload(chunk_t *c) {
    return (void *)((uintptr_t)c + HDR);
}

static inline chunk_t *from_payload(void *p) {
    return (chunk_t *)((uintptr_t)p - HDR);
}

static int bin_index(size_t size) {
    if (size < SMALL_BINS * 16)
        return (int)(size >> 4);

    int log = 63 - __builtin_clzl(size);
    int idx = SMALL_BINS + (log - 10) * 4 + (int)((size >> (log - 2)) & 3);
    return idx < NBINS ? idx : NBINS - 1;
}

static void bin_insert(chunk_t *c) {
    int i = bin_index(chunk_size(c));

    c->prev = NULL;
    c->next = bins[i];
    if (bins[i])
        bins[i]->prev = c;
    bins[i] = c;
    bin_map[i >> 6] |= 1ULL << (i & 63);
}

static void bin_remove(chunk_t *c) {
    int i = bin_index(chunk_size(c));

    if (c->prev)
        c->prev->next = c->next;
    else
        bins[i] = c->next;
    if (c->next)
        c->next->prev = c->prev;

    if (!bins[i])
        bin_map[i >> 6] &= ~(1ULL << (i & 63));
}

static void make_free(chunk_t *c, size_t size) {
    c->size = size | (c->size & PREV_INUSE);

    chunk_t *next = chunk_at(c, size);
    next->prev_size = size;
    next->size &= ~(size_t)PREV_INUSE;

    bin_insert(c);
}

// c is free and out of its bin, merge with whatever free neighbours it has then bin it
static void release(chunk_t *c) {
    size_t size = chunk_size(c);

    if (!(c->size & PREV_INUSE)) {
        chunk_t *prev = chunk_at(c, -(intptr_t)c->prev_size);
        bin_remove(prev);
        size += chunk_size(prev);
        c = prev;
    }

    chunk_t *next = chunk_at(c, size);
    if (!(next->size & INUSE)) {
        bin_remove(next);
        size += chunk_size(next);
    }

    make_free(c, size);
}

static void trim(chunk_t *c, size_t need) {
    size_t size = chunk_size(c);
    if (size - need < MIN_CHUNK)
        return;

    c->size = need | (c->size & (PREV_INUSE | INUSE));

    chunk_t *rest = chunk_at(c, need);
    rest->size = (size - need) | PREV_INUSE;
    release(rest);
}

static void take(chunk_t *c, size_t need) {
    bin_remove(c);
    c->size |= INUSE;
    chunk_at(c, chunk_size(c))->size |= PREV_INUSE;
    trim(c, need);
}

static chunk_t *find_fit(size_t need) {
    int i = bin_index(need);

    // the bin need lands in can hold smaller chunks too, so that one gets looked through
    for (chunk_t *c = bins[i]; c; c = c->next) {
        if (chunk_size(c) >= need)
            return c;
    }

    // every chunk in a later bin is big enough, take the first bin with anything in it
    for (int w = (i + 1) >> 6; w < NBINS / 64; w++) {
        uint64_t bits = bin_map[w];
        if (w == (i + 1) >> 6)
            bits &= ~0ULL << ((i + 1) & 63);
        if (bits)
            return bins[w * 64 + __builtin_ctzl(bits)];
    }

    return NULL;
}

// a fresh arena is one big free chunk with an in use fence after it so nothing merges off the end
static int grow(size_t need) {
    size_t bytes = need + HDR + MIN_CHUNK;
    if (bytes < ARENA_SIZE)
        bytes = ARENA_SIZE;
    size_t pages = (bytes + PAGE_SIZE - 1) / PAGE_SIZE;

    void *mem = _mmap(pages);
    if (!mem || (intptr_t)mem < 0)
        return -1;

    size_t span = pages * PAGE_SIZE - HDR;
    chunk_t *c = mem;
    c->prev_size = 0;
    c->size = PREV_INUSE;

    chunk_t *fence = chunk_at(c, span);
    fence->size = INUSE;

    make_free(c, span);
    return 0;
}

static size_t request_size(size_t n) {
    if (n > (size_t)-1 - HDR - 15)
        return 0;
    size_t need = (n + HDR + 15) & ~(size_t)15;
    return need < MIN_CHUNK ? MIN_CHUNK : need;
}

void *malloc(size_t n) {
    if (!n)
        return NULL;

    size_t need = request_size(n);
    if (!need)
        return NULL;

    if (need >= MMAP_THRESHOLD) {
        size_t pages = (need + PAGE_SIZE - 1) / PAGE_SIZE;
        chunk_t *c = _mmap(pages);
        if (!c || (intptr_t)c < 0)
            return NULL;
        c->prev_size = 0;
        c->size = (pages * PAGE_SIZE) | MMAPPED | INUSE;
        return payload(c);
    }

    lock_take(&__malloc_lock);
    chunk_t *c = find_fit(need);
    if (!c) {
        if (grow(need) < 0) {
            lock_give(&__malloc_lock);
            return NULL;
        }
        c = find_fit(need);
    }

    take(c, need);
    lock_give(&__malloc_lock);
    return payload(c);
}

void free(void *p) {
    if (!p)
        return;

    chunk_t *c = from_payload(p);
    if (c->size & MMAPPED) {
        _munmap((char *)c - c->prev_size);
        return;
    }

    lock_take(&__malloc_lock);
    c->size &= ~(size_t)INUSE;
    release(c);
    lock_give(&__malloc_lock);
}

void *realloc(void *p, size_t n) {
    if (!p)
        return malloc(n);
    if (!n) {
        free(p);
        return NULL;
    }

    size_t need = request_size(n);
    if (!need)
        return NULL;

    chunk_t *c = from_payload(p);
    size_t size = chunk_size(c);

    if (c->size & MMAPPED) {
        if (need <= size)
            return p;
    } else {
        lock_take(&__malloc_lock);
        if (need <= size) {
            trim(c, need);
            lock_give(&__malloc_lock);
            return p;
        }

        // grow in place when the chunk after is free and big enough
        chunk_t *next = chunk_at(c, size);
        if (!(next->size & INUSE) && size + chunk_size(next) >= need) {
            bin_remove(next);
            c->size += chunk_size(next);
            chunk_at(c, chunk_size(c))->size |= PREV_INUSE;
            trim(c, need);
            lock_give(&__malloc_lock);
            return p;
        }
        lock_give(&__malloc_lock);
    }

    void *np = malloc(n);
    if (!np)
        return NULL;
    memcpy(np, p, size - HDR);
    free(p);
    return np;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total;
    if (__builtin_mul_overflow(nmemb, size, &total))
        return NULL;

    void *p = malloc(total);
    if (p)
        memset(p, 0, total);
    return p;
}

// alignment past 16
void *memalign(size_t align, size_t n) {
    if (align <= 16)
        return malloc(n);
    if (align & (align - 1))
        return NULL;

    size_t need = request_size(n);
    if (!need || need > (size_t)-1 - align - MIN_CHUNK - PAGE_SIZE)
        return NULL;

    if (need + align >= MMAP_THRESHOLD) {
        size_t pages = (need + align + PAGE_SIZE - 1) / PAGE_SIZE;
        uintptr_t mem = (uintptr_t)_mmap(pages);
        if (!mem || (intptr_t)mem < 0)
            return NULL;
        uintptr_t pay = (mem + HDR + align - 1) & ~(uintptr_t)(align - 1);
        chunk_t *c = (chunk_t *)(pay - HDR);
        c->prev_size = (uintptr_t)c - mem;
        c->size = (pages * PAGE_SIZE - c->prev_size) | MMAPPED | INUSE;
        return payload(c);
    }

    void *p = malloc(need + align + MIN_CHUNK);
    if (!p)
        return NULL;

    lock_take(&__malloc_lock);
    chunk_t *c = from_payload(p);
    if (((uintptr_t)p & (align - 1)) == 0) {
        trim(c, need);
        lock_give(&__malloc_lock);
        return p;
    }

    uintptr_t ap    = ((uintptr_t)p + MIN_CHUNK + align - 1) & ~(uintptr_t)(align - 1);
    chunk_t  *ac    = from_payload((void *)ap);
    size_t    lead  = (uintptr_t)ac - (uintptr_t)c;
    size_t    total = chunk_size(c);

    ac->size = (total - lead) | INUSE;
    c->size  = lead | (c->size & PREV_INUSE);
    release(c);
    trim(ac, need);
    lock_give(&__malloc_lock);
    return (void *)ap;
}

void *aligned_alloc(size_t align, size_t n) {
    return memalign(align, n);
}

int posix_memalign(void **out, size_t align, size_t n) {
    if (align < sizeof(void *) || (align & (align - 1)))
        return EINVAL;
    void *p = memalign(align, n ? n : 1);
    if (!p)
        return ENOMEM;
    *out = p;
    return 0;
}

void *valloc(size_t n) {
    return memalign(PAGE_SIZE, n);
}

size_t malloc_usable_size(void *p) {
    if (!p)
        return 0;
    chunk_t *c = from_payload(p);
    return chunk_size(c) - HDR;
}
