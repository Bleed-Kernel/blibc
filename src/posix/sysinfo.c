// sysinfo and get_nprocs from the kernels meminfo and the affinity mask
#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <bleed/syscalls.h>
#include <bleed/sysinfo.h>

int get_nprocs_conf(void) {
    cpu_set_t set;
    if (sched_getaffinity(0, sizeof(set), &set) != 0)
        return 1;
    int n = CPU_COUNT(&set);
    return n > 0 ? n : 1;
}

int get_nprocs(void) {
    return get_nprocs_conf();
}

long get_phys_pages(void) {
    system_memory_info_t mem;
    if (_meminfo(&mem) != 0)
        return -1;
    return (long)(mem.MEMORY_USABLE / 4096);
}

long get_avphys_pages(void) {
    system_memory_info_t mem;
    if (_meminfo(&mem) != 0)
        return -1;
    return (long)(mem.MEMORY_FREE / 4096);
}

int sysinfo(struct sysinfo *info) {
    system_memory_info_t mem;
    if (!info) {
        errno = EFAULT;
        return -1;
    }
    memset(info, 0, sizeof(*info));
    if (_meminfo(&mem) != 0) {
        errno = EIO;
        return -1;
    }
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    info->uptime   = (unsigned long)ts.tv_sec;
    info->totalram = mem.MEMORY_USABLE;
    info->freeram  = mem.MEMORY_FREE;
    info->procs    = 1;
    info->mem_unit = 1;
    return 0;
}
