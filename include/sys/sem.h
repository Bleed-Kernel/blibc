#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ipc.h>
#include <time.h>

// system v semaphores, bleed has none so semget says ENOSYS
#define SEM_UNDO 0x1000
#define GETPID   11
#define GETVAL   12
#define GETALL   13
#define GETNCNT  14
#define GETZCNT  15
#define SETVAL   16
#define SETALL   17

struct sembuf {
    unsigned short sem_num;
    short          sem_op;
    short          sem_flg;
};

struct semid_ds {
    struct ipc_perm sem_perm;
    time_t          sem_otime;
    long            __unused1;
    time_t          sem_ctime;
    long            __unused2;
    unsigned long   sem_nsems;
    long            __unused3[2];
};

int semget(key_t key, int nsems, int flags);
int semop(int id, struct sembuf *ops, size_t n);
int semctl(int id, int num, int cmd, ...);

#ifdef __cplusplus
}
#endif
