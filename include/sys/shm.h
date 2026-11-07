#pragma once

#include <sys/ipc.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHMLBA 4096

#define SHM_RDONLY 010000
#define SHM_RND    020000
#define SHM_REMAP  040000
#define SHM_EXEC   0100000
#define SHM_LOCK   11
#define SHM_UNLOCK 12
#define SHM_STAT   13
#define SHM_INFO   14
#define SHM_HUGETLB 04000
#define SHM_NORESERVE 010000

typedef unsigned long shmatt_t;

struct shmid_ds {
    struct ipc_perm shm_perm;
    size_t shm_segsz;
    time_t shm_atime;
    time_t shm_dtime;
    time_t shm_ctime;
    pid_t shm_cpid;
    pid_t shm_lpid;
    unsigned long shm_nattch;
    unsigned long __pad1;
    unsigned long __pad2;
};

// no sysv shared memory, memfd + mmap is the way on bleed. these all fail with enosys
int   shmget(key_t key, size_t size, int flag);
void *shmat(int id, const void *addr, int flag);
int   shmdt(const void *addr);
int   shmctl(int id, int cmd, struct shmid_ds *buf);

#ifdef __cplusplus
}
#endif
