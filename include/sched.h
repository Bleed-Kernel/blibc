#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>

#define SCHED_OTHER 0
#define SCHED_FIFO  1
#define SCHED_RR    2
#define SCHED_BATCH 3
#define SCHED_IDLE  5

struct sched_param {
    int sched_priority;
};

// linuxs size, room for 1024 cpus
typedef struct {
    unsigned long __bits[128 / sizeof(long)];
} cpu_set_t;

#define __CPU_WORD(i) ((i) / (8 * sizeof(long)))
#define __CPU_BIT(i)  (1UL << ((i) % (8 * sizeof(long))))
#define CPU_SETSIZE   1024
#define CPU_ZERO(s)       __builtin_memset((s), 0, sizeof(cpu_set_t))
#define CPU_SET(i, s)     ((s)->__bits[__CPU_WORD(i)] |= __CPU_BIT(i))
#define CPU_CLR(i, s)     ((s)->__bits[__CPU_WORD(i)] &= ~__CPU_BIT(i))
#define CPU_ISSET(i, s)   (((s)->__bits[__CPU_WORD(i)] & __CPU_BIT(i)) != 0)
#define CPU_COUNT(s)      __sched_cpucount(sizeof(cpu_set_t), (s))
#define CPU_COUNT_S(n, s) __sched_cpucount((n), (s))
#define CPU_ALLOC_SIZE(n) ((((n) + 63) / 64) * 8)

int __sched_cpucount(size_t size, const cpu_set_t *set);
int sched_getaffinity(pid_t pid, size_t size, cpu_set_t *set);
int sched_setaffinity(pid_t pid, size_t size, const cpu_set_t *set);
int sched_getcpu(void);

int sched_yield(void);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_getparam(pid_t pid, struct sched_param *param);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_getscheduler(pid_t pid);
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);

#define CPU_ALLOC(n)          ((cpu_set_t *)__builtin_calloc(1, CPU_ALLOC_SIZE(n)))
#define CPU_FREE(s)           __builtin_free(s)
#define CPU_ZERO_S(n, s)      __builtin_memset((s), 0, (n))
#define CPU_SET_S(i, n, s)    ((size_t)(i) / 8 < (n) ? ((s)->__bits[__CPU_WORD(i)] |= __CPU_BIT(i)) : 0)
#define CPU_CLR_S(i, n, s)    ((size_t)(i) / 8 < (n) ? ((s)->__bits[__CPU_WORD(i)] &= ~__CPU_BIT(i)) : 0)
#define CPU_ISSET_S(i, n, s)  ((size_t)(i) / 8 < (n) ? (((s)->__bits[__CPU_WORD(i)] & __CPU_BIT(i)) != 0) : 0)
#define CPU_EQUAL(a, b)       (__builtin_memcmp((a), (b), sizeof(cpu_set_t)) == 0)
#define CPU_EQUAL_S(n, a, b)  (__builtin_memcmp((a), (b), (n)) == 0)

#ifdef __cplusplus
}
#endif
