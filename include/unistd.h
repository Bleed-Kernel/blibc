#pragma once

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
int close(int fd);
int dup2(int oldfd, int newfd);
int pipe(int fds[2]);
off_t lseek(int fd, off_t offset, int whence);
int chdir(const char *path);
int unlink(const char *path);
int rmdir(const char *path);
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
int access(const char *path, int mode);
void _exit(int code);

pid_t fork(void);
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
extern int optind, opterr, optopt;
