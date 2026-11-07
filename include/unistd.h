#pragma once

#define _POSIX_VERSION          200809L
#define _POSIX_THREADS          200809L
#define _POSIX_TIMERS           200809L
#define _POSIX_MONOTONIC_CLOCK  200809L
#define _POSIX_TIMEOUTS         200809L
#define _POSIX_SEMAPHORES       200809L
#define _POSIX_SPAWN            200809L

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stddef.h>
#include <bleed/ipc.h>

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

ssize_t read(int fd, void *buf, size_t len);
ssize_t write(int fd, const void *buf, size_t len);
ssize_t pread(int fd, void *buf, size_t len, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t len, off_t offset);
#define pread64 pread
#define pwrite64 pwrite
int close(int fd);
int dup2(int oldfd, int newfd);
int pipe(int fds[2]);
int pipe2(int fds[2], int flags);
int ftruncate(int fd, off_t length);
int truncate(const char *path, off_t length);

// no links of either kind, these fail with EPERM like a filesystem without them would
int link(const char *from, const char *to);
int linkat(int fromfd, const char *from, int tofd, const char *to, int flags);
int symlink(const char *target, const char *path);
int symlinkat(const char *target, int dirfd, const char *path);
ssize_t readlinkat(int dirfd, const char *path, char *buf, size_t size);

// no sessions or process groups yet, every process is its own of both
pid_t setsid(void);
pid_t getsid(pid_t pid);
int   setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);

int chown(const char *path, uid_t uid, gid_t gid);
int fchown(int fd, uid_t uid, gid_t gid);
int lchown(const char *path, uid_t uid, gid_t gid);

#define _PC_LINK_MAX    0
#define _PC_MAX_CANON   1
#define _PC_MAX_INPUT   2
#define _PC_NAME_MAX    3
#define _PC_PATH_MAX    4
#define _PC_PIPE_BUF    5
long pathconf(const char *path, int name);
long fpathconf(int fd, int name);
int dup3(int oldfd, int newfd, int flags);
off_t lseek(int fd, off_t offset, int whence);
int chdir(const char *path);
int unlink(const char *path);
int rmdir(const char *path);
int fsync(int fd);
unsigned int sleep(unsigned int seconds);
int usleep(unsigned long usec);
char *getcwd(char *buf, size_t size);
int isatty(int fd);
pid_t getpid(void);
pid_t getppid(void);

#define _SC_CLK_TCK             2
#define _SC_OPEN_MAX            4
#define _SC_PAGESIZE            30
#define _SC_PAGE_SIZE           _SC_PAGESIZE
#define _SC_NPROCESSORS_CONF    83
#define _SC_NPROCESSORS_ONLN    84
#define _SC_PHYS_PAGES          85
#define _SC_HOST_NAME_MAX       180
long sysconf(int name);
int dup(int fd);
char *ttyname(int fd);
int gethostname(char *name, size_t len);
ssize_t readlink(const char *path, char *buf, size_t size);

// theres one user and its root
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int setuid(uid_t uid);
int setgid(gid_t gid);
int seteuid(uid_t uid);
int setegid(gid_t gid);
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
int getgroups(int size, gid_t list[]);
int setgroups(size_t size, const gid_t *list);
int chroot(const char *path);
int pause(void);
long syscall(long n, ...);
int access(const char *path, int mode);
void _exit(int code);

pid_t fork(void);
// a fork the parent waits on till the child execs or exits, memory isnt actually shared
pid_t vfork(void);
int execve(const char *path, char *const argv[], char *const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
int execl(const char *path, const char *arg0, ...);
int execlp(const char *file, const char *arg0, ...);
int execle(const char *path, const char *arg0, ...);

int ipc_send(pid_t target_pid, void *addr, size_t pages);
int ipc_recv(ipc_message_t *msg);

extern char **environ;

int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
int getentropy(void *buf, size_t len);
extern int optind, opterr, optopt;

int getpagesize(void);
// SIGALRM after secs seconds, 0 cancels
unsigned alarm(unsigned secs);
unsigned ualarm(unsigned usecs, unsigned interval);

#ifdef __cplusplus
}
#endif
