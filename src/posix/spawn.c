#include <spawn.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

enum { FA_OPEN, FA_CLOSE, FA_DUP2, FA_CHDIR, FA_FCHDIR };

typedef struct {
    int    op;
    int    fd;
    int    newfd;
    int    flags;
    mode_t mode;
    char  *path;
} file_action_t;

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *fa) {
    fa->count   = 0;
    fa->cap     = 0;
    fa->actions = NULL;
    return 0;
}

int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *fa) {
    file_action_t *a = fa->actions;
    for (int i = 0; i < fa->count; i++)
        free(a[i].path);
    free(a);
    fa->actions = NULL;
    fa->count   = fa->cap = 0;
    return 0;
}

static file_action_t *add_action(posix_spawn_file_actions_t *fa) {
    if (fa->count == fa->cap) {
        int cap = fa->cap ? fa->cap * 2 : 8;
        file_action_t *more = realloc(fa->actions, (size_t)cap * sizeof(file_action_t));
        if (!more)
            return NULL;
        fa->actions = more;
        fa->cap     = cap;
    }
    file_action_t *a = (file_action_t *)fa->actions + fa->count++;
    memset(a, 0, sizeof(*a));
    return a;
}

int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *fa, int fd, const char *path, int flags, mode_t mode) {
    if (fd < 0)
        return EBADF;
    char *copy = strdup(path);
    file_action_t *a = copy ? add_action(fa) : NULL;
    if (!a) {
        free(copy);
        return ENOMEM;
    }
    *a = (file_action_t){ .op = FA_OPEN, .fd = fd, .flags = flags, .mode = mode, .path = copy };
    return 0;
}

int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *fa, int fd) {
    if (fd < 0)
        return EBADF;
    file_action_t *a = add_action(fa);
    if (!a)
        return ENOMEM;
    *a = (file_action_t){ .op = FA_CLOSE, .fd = fd };
    return 0;
}

int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *fa, int fd, int newfd) {
    if (fd < 0 || newfd < 0)
        return EBADF;
    file_action_t *a = add_action(fa);
    if (!a)
        return ENOMEM;
    *a = (file_action_t){ .op = FA_DUP2, .fd = fd, .newfd = newfd };
    return 0;
}

int posix_spawn_file_actions_addchdir_np(posix_spawn_file_actions_t *fa, const char *path) {
    char *copy = strdup(path);
    file_action_t *a = copy ? add_action(fa) : NULL;
    if (!a) {
        free(copy);
        return ENOMEM;
    }
    *a = (file_action_t){ .op = FA_CHDIR, .path = copy };
    return 0;
}

int posix_spawn_file_actions_addfchdir_np(posix_spawn_file_actions_t *fa, int fd) {
    file_action_t *a = add_action(fa);
    if (!a)
        return ENOMEM;
    *a = (file_action_t){ .op = FA_FCHDIR, .fd = fd };
    return 0;
}

int posix_spawnattr_init(posix_spawnattr_t *attr) {
    memset(attr, 0, sizeof(*attr));
    return 0;
}

int posix_spawnattr_destroy(posix_spawnattr_t *attr) {
    (void)attr;
    return 0;
}

int posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags) {
    if (flags & ~0xff)
        return EINVAL;
    attr->flags = flags;
    return 0;
}

int posix_spawnattr_getflags(const posix_spawnattr_t *attr, short *flags) {
    *flags = attr->flags;
    return 0;
}

int posix_spawnattr_setpgroup(posix_spawnattr_t *attr, pid_t pgroup) {
    attr->pgroup = pgroup;
    return 0;
}

int posix_spawnattr_getpgroup(const posix_spawnattr_t *attr, pid_t *pgroup) {
    *pgroup = attr->pgroup;
    return 0;
}

int posix_spawnattr_setsigmask(posix_spawnattr_t *attr, const sigset_t *mask) {
    attr->sigmask = *mask;
    return 0;
}

int posix_spawnattr_getsigmask(const posix_spawnattr_t *attr, sigset_t *mask) {
    *mask = attr->sigmask;
    return 0;
}

int posix_spawnattr_setsigdefault(posix_spawnattr_t *attr, const sigset_t *def) {
    attr->sigdefault = *def;
    return 0;
}

int posix_spawnattr_getsigdefault(const posix_spawnattr_t *attr, sigset_t *def) {
    *def = attr->sigdefault;
    return 0;
}

int posix_spawnattr_setschedparam(posix_spawnattr_t *attr, const struct sched_param *param) {
    attr->sched = *param;
    return 0;
}

int posix_spawnattr_getschedparam(const posix_spawnattr_t *attr, struct sched_param *param) {
    *param = attr->sched;
    return 0;
}

int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attr, int policy) {
    attr->policy = policy;
    return 0;
}

int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *attr, int *policy) {
    *policy = attr->policy;
    return 0;
}

static int child_setup(const posix_spawn_file_actions_t *fa, const posix_spawnattr_t *attr) {
    if (attr && (attr->flags & POSIX_SPAWN_SETSIGDEF)) {
        for (int sig = 1; sig < NSIG; sig++) {
            if (sigismember(&attr->sigdefault, sig) == 1)
                signal(sig, SIG_DFL);
        }
    }
    if (attr && (attr->flags & POSIX_SPAWN_SETSIGMASK))
        sigprocmask(SIG_SETMASK, &attr->sigmask, NULL);

    const file_action_t *a = fa ? fa->actions : NULL;
    for (int i = 0; fa && i < fa->count; i++) {
        switch (a[i].op) {
            case FA_OPEN: {
                int fd = open(a[i].path, a[i].flags, a[i].mode);
                if (fd < 0)
                    return errno;
                if (fd != a[i].fd) {
                    if (dup2(fd, a[i].fd) < 0)
                        return errno;
                    close(fd);
                }
                break;
            }
            case FA_CLOSE:
                close(a[i].fd);
                break;
            case FA_DUP2:
                // dup2 onto itself is how you ask for it to survive exec
                if (a[i].fd == a[i].newfd) {
                    int flags = fcntl(a[i].fd, F_GETFD);
                    if (flags < 0 || fcntl(a[i].fd, F_SETFD, flags & ~FD_CLOEXEC) < 0)
                        return errno;
                } else if (dup2(a[i].fd, a[i].newfd) < 0) {
                    return errno;
                }
                break;
            case FA_CHDIR:
                if (chdir(a[i].path) < 0)
                    return errno;
                break;
            case FA_FCHDIR:
                return ENOSYS;
        }
    }
    return 0;
}

static int spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *fa,
                 const posix_spawnattr_t *attr, char *const argv[], char *const envp[], int search) {
    int p[2];
    if (pipe2(p, O_CLOEXEC) < 0)
        return errno;

    pid_t child = vfork();
    if (child < 0) {
        int err = errno;
        close(p[0]);
        close(p[1]);
        return err;
    }

    if (child == 0) {
        close(p[0]);
        // out of the way of whatever the file actions dup2 onto
        int report = fcntl(p[1], F_DUPFD_CLOEXEC, 200);
        if (report >= 0)
            close(p[1]);
        else
            report = p[1];

        int err = child_setup(fa, attr);
        if (!err) {
            char *const *env = envp ? envp : environ;
            if (search)
                execvpe(path, argv, env);
            else
                execve(path, argv, env);
            err = errno;
        }
        write(report, &err, sizeof(err));
        _exit(127);
    }

    close(p[1]);
    int  err = 0;
    long n   = read(p[0], &err, sizeof(err));
    close(p[0]);
    if (n == (long)sizeof(err) && err) {
        waitpid(child, NULL, 0);
        return err;
    }
    if (pid)
        *pid = child;
    return 0;
}

int posix_spawn(pid_t *pid, const char *path, const posix_spawn_file_actions_t *fa,
                const posix_spawnattr_t *attr, char *const argv[], char *const envp[]) {
    return spawn(pid, path, fa, attr, argv, envp, 0);
}

int posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *fa,
                 const posix_spawnattr_t *attr, char *const argv[], char *const envp[]) {
    return spawn(pid, file, fa, attr, argv, envp, 1);
}
