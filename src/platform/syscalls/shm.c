#include <abi/syscalls.h>
#include <stdint.h>
#include <stddef.h>

static inline void* syscall_shm_create(size_t pages, uint64_t *out_id) {
    void* ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SHM_CREATE), "D"(pages), "S"(out_id)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline void* syscall_shm_map(uint64_t id) {
    void* ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SHM_MAP), "D"(id)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall_shm_unmap(void *addr) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SHM_UNMAP), "D"(addr)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall_shm_destroy(uint64_t id) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SHM_DESTROY), "D"(id)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void* _shm_create(size_t pages, uint64_t *out_id) {
    return syscall_shm_create(pages, out_id);
}

void* _shm_map(uint64_t id) {
    return syscall_shm_map(id);
}

long _shm_unmap(void *addr) {
    return syscall_shm_unmap(addr);
}

long _shm_destroy(uint64_t id) {
    return syscall_shm_destroy(id);
}
