#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Raw bleed kernel syscalls: the syscall numbers and one thin `_name` wrapper
// per syscall (implemented in src/syscalls.c). These return the kernel's value
// untouched and never set errno; the POSIX wrappers in <unistd.h> etc. do that.

#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <bleed/ipc.h>
#include <bleed/scheduler.h>
#include <bleed/sysinfo.h>

enum {
    SYS_READ            = 0,
    SYS_WRITE           = 1,
    SYS_OPEN            = 2,
    SYS_CLOSE           = 3,
    SYS_STAT            = 4,
    SYS_FSTAT           = 5,
    SYS_POLL            = 7,
    SYS_SEEK            = 8,
    SYS_MMAP            = 9,
    SYS_MPROTECT        = 10,
    SYS_MUNMAP          = 11,
    SYS_IOCTL           = 16,
    SYS_DUP             = 32,
    SYS_DUP2            = 33,
    SYS_FORK            = 57,
    SYS_EXEC            = 59,
    SYS_EXIT            = 60,
    SYS_WAITPID         = 61,
    SYS_KILL            = 62,
    SYS_MKDIR           = 83,
    SYS_UNLINK          = 87,
    SYS_RENAME          = 82,
    SYS_GETCWD          = 79,
    SYS_CHDIR           = 80,
    SYS_READDIR         = 89,
    SYS_PIPE            = 293,
    SYS_GETPID          = 39,
    SYS_GETPPID         = 110,
    SYS_OPENAT          = 257,
    SYS_FSTATAT         = 262,
    SYS_SIGACTION       = 13,
    SYS_SIGPROCMASK     = 14,
    SYS_SIGRETURN       = 15,
    SYS_TIME            = 201,
    SYS_ARCH_PRCTL      = 158,
    SYS_EPOLL_CREATE1   = 291,
    SYS_EPOLL_CTL       = 233,
    SYS_EPOLL_WAIT      = 232,
    SYS_EPOLL_PWAIT     = 281,

    // bleed kernel specific system calls
    SYS_YIELD           = 512,
    SYS_SPAWN           = 513,
    SYS_MEMINFO         = 516,
    SYS_TASKCOUNT       = 517,
    SYS_TASKINFO        = 518,
    SYS_MAPFB           = 519,
    SYS_IPC_SEND        = 520,
    SYS_IPC_RECV        = 521,
    SYS_TASKUSAGE       = 525,
    SYS_TASKPATH        = 526,
    SYS_EXECVE          = 528,
    SYS_WAITSTATUS      = 529,
    SYS_MOUNTINFO       = 530,
};


struct bleed_stat {
    uint64_t dev;
    uint64_t ino;
    uint32_t mode;
    int32_t  uid;
    int32_t  gid;
    uint32_t pad;
    uint64_t nlink;
    uint64_t rdev;
    int64_t  size;
    int64_t  blksize;
    int64_t  blocks;
    int64_t  atime;
    int64_t  mtime;
    int64_t  ctime;
};

typedef struct syscall_dirent {
    char name[256];
    int type;
} dirent_t;

typedef struct rtc_time {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t mon;
    uint16_t year;
} rtc_time_t;

// files
int _open(const char *path, int flags);
int _close(int fd);
long _read(int fd, void *buf, unsigned long len);
long _write(int fd, const void *buf, long len);
long _seek(int fd, long offset, int whence);
struct pollfd;
long _poll(struct pollfd *fds, unsigned long nfds, long timeout_ms);
int _ioctl(int fd, unsigned long request, void* arg);
long _dup2(int oldfd, int newfd);
long _dup(int fd);
int _openat(int dirfd, const char *path, int flags);
long _fstatat(int dirfd, const char *path, struct bleed_stat *st, int flags);
long _pipe(int fds[2]);
long _stat(const char *path, struct bleed_stat *st);
long _fstat(int fd, struct bleed_stat *st);
long _readdir(int fd, size_t index, dirent_t *out);
int _mkdir(const char *path, int mode);
int _unlink(const char *path);
int _rename(const char *oldpath, const char *newpath);
long _chdir(const char *path);
char *_getcwd(char *buf, long size);

// processes
void _exit(int code);
long _fork(void);
long _exec(const char *path, const char *const argv[], uint64_t argc);
long _execve(const char *path, const char *const argv[], uint64_t argc, const char *const envp[]);
long _waitpid(long pid);
long _waitstatus(long pid, int *status);
long _arch_prctl(long code, uint64_t addr);
long _getpid(void);
long _getppid(void);
long _kill(long pid, long sig);
void _yeild();
uint64_t _taskcount(void);
uint64_t _taskinfo(uint64_t pid, user_task_info_t *info);
long _taskusage(uint64_t pid, user_task_usage_t *usage);
long _taskpath(uint64_t pid, char *buf, uint64_t size);

// signals
long _sigaction(int sig, const sigaction_t *act, sigaction_t *oldact);
long _sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
long _sigreturn(void);

// memory
void* _mmap(unsigned long pages);
void _munmap(void* addr);
long _mprotect(void *addr, unsigned long len, int prot);
void* mapfb(size_t *out_pages);
int _meminfo(system_memory_info_t *buf);
// -ENOENT once index is past the last mount
long _mountinfo(uint64_t index, user_mount_info_t *info);

// ipc
long _ipc_send(uint64_t target_pid, uint64_t src_addr, uint64_t pages);
long _ipc_recv(ipc_message_t *msg);

// time
int _time(rtc_time_t *buf);

#ifdef __cplusplus
}
#endif
