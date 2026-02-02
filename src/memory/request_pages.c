#include <libc/stdlib.h>
#include <libc/syscalls/mmap.h>
#include "memory_internal.h"

block_t* request_pages(size_t size) {
    size_t total = size + sizeof(block_t);
    size_t pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;

    void* mem = _mmap(pages);
    if (!mem)
        return NULL;

    block_t* b = (block_t*)mem;
    b->size = pages * PAGE_SIZE - sizeof(block_t);
    b->free = 0;
    b->next = NULL;

    return b;
}
