#include <libc/signal.h>

#include <libc/errno.h>
#include <libc/stdint.h>
#include <libc/syscalls/getpid.h>
#include <libc/syscalls/kill.h>
#include <libc/syscalls/signals.h>
#include <libc/syscalls/sigreturn.h>

#include "compat_errno.h"

static int signal_valid(int sig) {
    return sig > 0 && sig < NSIG;
}

static sigset_t sig_bit(int sig) {
    return (sigset_t)1ULL << (sig - 1);
}

static sigset_t valid_sig_mask(void) {
    return ((sigset_t)1ULL << (NSIG - 1)) - 1ULL;
}

__attribute__((noreturn))
static void __blibc_sigreturn_restorer(void) {
    (void)_sigreturn();
    for (;;) { }
}

int kill(pid_t pid, int sig) {
    long ret = _kill((long)pid, (long)sig);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact) {
    sigaction_t kact;
    const sigaction_t *act_ptr = NULL;
    long ret;

    if (act) {
        kact = *(const sigaction_t *)act;
        if (kact.handler != SIG_DFL && kact.handler != SIG_IGN && kact.restorer < 2)
            kact.restorer = (uintptr_t)__blibc_sigreturn_restorer;
        act_ptr = &kact;
    }

    ret = _sigaction(sig, act_ptr, (sigaction_t *)oldact);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    long ret = _sigprocmask(how, set, oldset);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return (int)ret;
}

sighandler_t signal(int sig, sighandler_t handler) {
    struct sigaction act;
    struct sigaction old;

    act.handler = (uintptr_t)handler;
    act.mask = 0;
    act.flags = 0;
    act.restorer = 0;

    if (sigaction(sig, &act, &old) < 0)
        return SIG_ERR;

    return (sighandler_t)old.handler;
}

int raise(int sig) {
    long pid = _getpid();
    if (pid < 0)
        return (int)posix_errno_from_ret(pid);
    return kill((pid_t)pid, sig);
}

int sigemptyset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    *set = valid_sig_mask();
    return 0;
}

int sigaddset(sigset_t *set, int sig) {
    if (!set || !signal_valid(sig)) {
        errno = EINVAL;
        return -1;
    }
    *set |= sig_bit(sig);
    return 0;
}

int sigdelset(sigset_t *set, int sig) {
    if (!set || !signal_valid(sig)) {
        errno = EINVAL;
        return -1;
    }
    *set &= ~sig_bit(sig);
    return 0;
}

int sigismember(const sigset_t *set, int sig) {
    if (!set || !signal_valid(sig)) {
        errno = EINVAL;
        return -1;
    }
    return ((*set & sig_bit(sig)) != 0) ? 1 : 0;
}
