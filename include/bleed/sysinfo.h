#pragma once

#include <stdint.h>

typedef struct system_memory_info{
    uint64_t MEMORY_USABLE;
    uint64_t MEMORY_RESERVED;
    uint64_t MEMORY_FAULTY;
    uint64_t MEMORY_FREE;
    uint64_t MEMORY_KERNEL;
    uint64_t MEMORY_BOOTLOADER;
    uint64_t MEMORY_ACPI;
    uint64_t MEMORY_FRAMEBUFFER;
    uint64_t MEMORY_OTHER;
} system_memory_info_t;

typedef struct user_mount_info {
    char     path[256];
    char     fs[16];
    char     device[64];
    uint64_t block_size;
    uint64_t blocks_total;
    uint64_t blocks_free;
} user_mount_info_t;
