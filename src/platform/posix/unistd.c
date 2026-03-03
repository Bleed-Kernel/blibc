#include <libc/unistd.h>

#include <libc/devices/console.h>
#include <libc/errno.h>
#include <libc/stdarg.h>
#include <libc/string.h>
#include <libc/syscalls/chdir.h>
#include <libc/syscalls/close.h>
#include <libc/syscalls/dup2.h>
#include <libc/syscalls/exec.h>
#include <libc/syscalls/fork.h>
#include <libc/syscalls/getcwd.h>
#include <libc/syscalls/getpid.h>
#include <libc/syscalls/ipc.h>
#include <libc/syscalls/ioctl.h>
#include <libc/syscalls/pipe.h>
#include <libc/syscalls/read.h>
#include <libc/syscalls/seek.h>
#include <libc/syscalls/write.h>

#include "compat_errno.h"

#define EXEC_MAX_ARGS 32
#define BLIBC_PATH_MAX 4096

static char *empty_environ[] = { NULL };
char **environ = empty_environ;

void __blibc_set_environ(char **envp) {
    if (envp)
        environ = envp;
    else
        environ = empty_environ;
}

static const char *lookup_env(const char *name, char *const envp[]) {
    char *const *env = envp ? envp : environ;
    if (!name || !*name || !env)
        return NULL;

    size_t name_len = strlen(name);
    for (; *env; env++) {
        if (strncmp(*env, name, name_len) == 0 && (*env)[name_len] == '=')
            return *env + name_len + 1;
    }

    return NULL;
}

static int exec_search_path(const char *file, char *const argv[], char *const envp[]) {
    if (!file || !*file || !argv) {
        errno = EFAULT;
        return -1;
    }

    if (strchr(file, '/'))
        return execve(file, argv, envp);

    const char *path = lookup_env("PATH", envp);
    if (!path || !*path)
        path = "/bin:/usr/bin";

    int saw_eacces = 0;

    while (*path) {
        const char *next = strchr(path, ':');
        size_t dir_len = next ? (size_t)(next - path) : strlen(path);
        const char *dir = path;

        if (dir_len == 0) {
            dir = ".";
            dir_len = 1;
        }

        size_t file_len = strlen(file);
        if (dir_len + 1 + file_len + 1 < BLIBC_PATH_MAX) {
            char candidate[BLIBC_PATH_MAX];
            memcpy(candidate, dir, dir_len);
            candidate[dir_len] = '/';
            memcpy(candidate + dir_len + 1, file, file_len + 1);

            execve(candidate, argv, envp);
            if (errno == EACCES)
                saw_eacces = 1;
            else if (errno != ENOENT && errno != ENOTDIR)
                return -1;
        }

        if (!next)
            break;
        path = next + 1;
    }

    errno = saw_eacces ? EACCES : ENOENT;
    return -1;
}

ssize_t read(int fd, void *buf, size_t len) {
    long ret = _read(fd, buf, (unsigned long)len);
    if (ret < 0) return posix_errno_from_ret(ret);
    return (ssize_t)ret;
}

ssize_t write(int fd, const void *buf, size_t len) {
    long ret = _write(fd, buf, (long)len);
    if (ret < 0) return posix_errno_from_ret(ret);
    return (ssize_t)ret;
}

int close(int fd) {
    long ret = _close(fd);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int dup2(int oldfd, int newfd) {
    long ret = _dup2(oldfd, newfd);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int pipe(int fds[2]) {
    if (!fds) {
        errno = EFAULT;
        return -1;
    }

    long ret = _pipe(fds);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

off_t lseek(int fd, off_t offset, int whence) {
    long ret = _seek(fd, (long)offset, whence);
    if (ret < 0) return (off_t)posix_errno_from_ret(ret);
    return (off_t)ret;
}

int chdir(const char *path) {
    long ret = _chdir(path);
    if (ret < 0) return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

char *getcwd(char *buf, size_t size) {
    char *ret = _getcwd(buf, (long)size);
    if (!ret) {
        errno = EIO;
        return NULL;
    }
    return ret;
}

int isatty(int fd) {
    int flags = 0;
    long ret = _ioctl(fd, TTY_IOCTL_GET_FLAGS, &flags);
    if (ret < 0) {
        errno = ENOTTY;
        return 0;
    }
    return 1;
}

pid_t getpid(void) {
    long ret = _getpid();
    if (ret < 0)
        return (pid_t)posix_errno_from_ret(ret);
    return (pid_t)ret;
}

int ipc_send(pid_t target_pid, void *addr, size_t pages) {
    long ret = _ipc_send((uint64_t)target_pid, (uint64_t)(uintptr_t)addr, (uint64_t)pages);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int ipc_recv(ipc_message_t *msg) {
    if (!msg) {
        errno = EFAULT;
        return -1;
    }

    long ret = _ipc_recv(msg);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

pid_t fork(void) {
    long ret = _fork();
    if (ret < 0)
        return (pid_t)posix_errno_from_ret(ret);
    return (pid_t)ret;
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    (void)envp;

    if (!path || !argv) {
        errno = EFAULT;
        return -1;
    }

    uint64_t argc = 0;
    while (argv[argc]) {
        argc++;
        if (argc > EXEC_MAX_ARGS) {
            errno = E2BIG;
            return -1;
        }
    }

    long ret = _exec(path, (const char *const *)argv, argc);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);

    return (int)ret;
}

int execv(const char *path, char *const argv[]) {
    return execve(path, argv, environ);
}

int execvp(const char *file, char *const argv[]) {
    return exec_search_path(file, argv, environ);
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    return exec_search_path(file, argv, envp);
}

int execl(const char *path, const char *arg0, ...) {
    if (!path || !arg0) {
        errno = EFAULT;
        return -1;
    }

    char *argv[EXEC_MAX_ARGS + 1];
    size_t argc = 0;

    argv[argc++] = (char *)arg0;

    va_list ap;
    va_start(ap, arg0);
    for (;;) {
        char *arg = va_arg(ap, char *);
        if (!arg)
            break;
        if (argc >= EXEC_MAX_ARGS) {
            va_end(ap);
            errno = E2BIG;
            return -1;
        }
        argv[argc++] = arg;
    }
    va_end(ap);

    argv[argc] = NULL;
    return execve(path, argv, environ);
}

int execlp(const char *file, const char *arg0, ...) {
    if (!file || !arg0) {
        errno = EFAULT;
        return -1;
    }

    char *argv[EXEC_MAX_ARGS + 1];
    size_t argc = 0;

    argv[argc++] = (char *)arg0;

    va_list ap;
    va_start(ap, arg0);
    for (;;) {
        char *arg = va_arg(ap, char *);
        if (!arg)
            break;
        if (argc >= EXEC_MAX_ARGS) {
            va_end(ap);
            errno = E2BIG;
            return -1;
        }
        argv[argc++] = arg;
    }
    va_end(ap);

    argv[argc] = NULL;
    return execvp(file, argv);
}

int execle(const char *path, const char *arg0, ...) {
    if (!path || !arg0) {
        errno = EFAULT;
        return -1;
    }

    char *argv[EXEC_MAX_ARGS + 1];
    size_t argc = 0;
    char *const *envp = NULL;

    argv[argc++] = (char *)arg0;

    va_list ap;
    va_start(ap, arg0);
    for (;;) {
        char *arg = va_arg(ap, char *);
        if (!arg)
            break;
        if (argc >= EXEC_MAX_ARGS) {
            va_end(ap);
            errno = E2BIG;
            return -1;
        }
        argv[argc++] = arg;
    }

    envp = va_arg(ap, char *const *);
    va_end(ap);

    argv[argc] = NULL;
    return execve(path, argv, (char *const *)envp);
}
