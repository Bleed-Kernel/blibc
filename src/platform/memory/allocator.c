#include "memory_internal.h"
#include <libc/string.h>
#include <libc/syscalls/mmap.h>
#include <libc/syscalls/munmap.h>

static block_t* heap_head = NULL;
static block_t* heap_tail = NULL;

static void coalesce() {
    block_t* cur = heap_head;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += sizeof(block_t) + cur->next->size;
            cur->next = cur->next->next;
            if (cur->next == NULL)
                heap_tail = cur;
        } else {
            cur = cur->next;
        }
    }
}

static void split_block(block_t* b, size_t size) {
    if (b->size <= size + sizeof(block_t))
        return;

    block_t* newb = (block_t*)((uintptr_t)(b + 1) + size);
    newb->size = b->size - size - sizeof(block_t);
    newb->free = 1;
    newb->next = b->next;

    b->size = size;
    b->next = newb;
}

void* malloc(size_t size) {
    if (!size)
        return NULL;

    block_t* cur = heap_head;
    while (cur) {
        if (cur->free && cur->size >= size) {
            split_block(cur, size);
            cur->free = 0;
            return (void*)(cur + 1);
        }
        cur = cur->next;
    }

    block_t* b = request_pages(size);
    if (!b)
        return NULL;

    if (!heap_head)
        heap_head = b;
    else
        heap_tail->next = b;

    heap_tail = b;
    return (void*)(b + 1);
}

void free(void* ptr) {
    if (!ptr)
        return;

    block_t* b = ((block_t*)ptr) - 1;
    b->free = 1;

    coalesce();
}

void* realloc(void* ptr, size_t size) {
    if (!ptr)
        return malloc(size);

    if (!size) {
        free(ptr);
        return NULL;
    }

    block_t* b = ((block_t*)ptr) - 1;
    if (b->size >= size)
        return ptr;

    void* newp = malloc(size);
    if (!newp)
        return NULL;

    memcpy(newp, ptr, b->size);
    free(ptr);
    return newp;
}

void* calloc(size_t nmemb, size_t size) {
    size_t total;
    if (__builtin_mul_overflow(nmemb, size, &total))
        return NULL;

    void* p = malloc(total);
    if (p)
        memset(p, 0, total);
    return p;
}