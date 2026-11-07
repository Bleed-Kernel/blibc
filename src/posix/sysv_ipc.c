// system v ipc, nothing in the kernel for it yet
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>

key_t ftok(const char *path, int id) {
    struct stat st;
    if (stat(path, &st) < 0)
        return -1;
    return (key_t)((st.st_ino & 0xffff) | ((st.st_dev & 0xff) << 16) | ((unsigned)(id & 0xff) << 24));
}

int semget(key_t key, int nsems, int flags) {
    (void)key; (void)nsems; (void)flags;
    errno = ENOSYS;
    return -1;
}

int semop(int id, struct sembuf *ops, size_t n) {
    (void)id; (void)ops; (void)n;
    errno = ENOSYS;
    return -1;
}

int semctl(int id, int num, int cmd, ...) {
    (void)id; (void)num; (void)cmd;
    errno = ENOSYS;
    return -1;
}
