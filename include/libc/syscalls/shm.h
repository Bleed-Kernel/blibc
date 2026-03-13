#pragma once

#include <libc/stdint.h>
#include <libc/stddef.h>

void* _shm_create(size_t pages, uint64_t *out_id);
void* _shm_map(uint64_t id);
long _shm_unmap(void *addr);
long _shm_destroy(uint64_t id);
