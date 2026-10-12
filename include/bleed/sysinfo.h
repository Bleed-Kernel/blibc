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
