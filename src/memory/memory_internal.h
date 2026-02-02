#pragma once
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

typedef struct block {
    size_t size;
    int free;
    struct block* next;
} block_t;

block_t* request_pages(size_t size);