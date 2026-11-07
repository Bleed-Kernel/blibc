#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);
void *memalign(size_t align, size_t size);
void *valloc(size_t size);
size_t malloc_usable_size(void *ptr);

#ifdef __cplusplus
}
#endif
