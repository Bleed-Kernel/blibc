#include <signal.h>

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

// the kernel knows the first 64 signals
static int signal_valid(int sig) {
    return sig > 0 && sig < NSIG;
}

static unsigned long sig_bit(int sig) {
    return 1UL << ((sig - 1) % (8 * sizeof(long)));
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
    sigaction_t kact, kold;
    const sigaction_t *act_ptr = NULL;

    if (act) {
        kact.handler  = act->handler;
        kact.mask     = act->sa_mask.__bits[0];
        kact.flags    = (unsigned int)act->sa_flags & ~(unsigned)SA_RESTORER;
        kact.restorer = (uintptr_t)__blibc_sigreturn_restorer;
        act_ptr = &kact;
    }

    long ret = _sigaction(sig, act_ptr, oldact ? &kold : NULL);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);

    if (oldact) {
        memset(oldact, 0, sizeof(*oldact));
        oldact->handler           = kold.handler;
        oldact->sa_mask.__bits[0] = kold.mask;
        oldact->sa_flags          = (int)kold.flags;
    }
    return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    unsigned long kset = set ? set->__bits[0] : 0, kold = 0;
    long ret = _sigprocmask(how, set ? (const void *)&kset : NULL, oldset ? (void *)&kold : NULL);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    if (oldset) {
        memset(oldset, 0, sizeof(*oldset));
        oldset->__bits[0] = kold;
    }
    return 0;
}

sighandler_t signal(int sig, sighandler_t handler) {
    struct sigaction act, old;

    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    act.sa_flags   = SA_RESTART;

    if (sigaction(sig, &act, &old) < 0)
        return SIG_ERR;
    return old.sa_handler;
}

int siginterrupt(int sig, int flag) {
    struct sigaction act;
    if (sigaction(sig, NULL, &act) < 0)
        return -1;
    if (flag)
        act.sa_flags &= ~SA_RESTART;
    else
        act.sa_flags |= SA_RESTART;
    return sigaction(sig, &act, NULL);
}

int raise(int sig) {
    long pid = _getpid();
    if (pid < 0)
        return (int)posix_errno_from_ret(pid);
    return kill((pid_t)pid, sig);
}

int sigaltstack(const stack_t *ss, stack_t *old) {
    long ret = _sigaltstack(ss, old);
    if (ret < 0)
        return (int)posix_errno_from_ret(ret);
    return 0;
}

// theres no atomic swap of the mask and sleep yet
int sigsuspend(const sigset_t *mask) {
    sigset_t old;
    sigprocmask(SIG_SETMASK, mask, &old);
    poll(NULL, 0, -1);
    sigprocmask(SIG_SETMASK, &old, NULL);
    errno = EINTR;
    return -1;
}

// the kernel doesnt say whats pending yet
int sigpending(sigset_t *set) {
    return sigemptyset(set);
}

int sigemptyset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0, sizeof(*set));
    return 0;
}

int sigfillset(sigset_t *set) {
    if (!set) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0, sizeof(*set));
    set->__bits[0] = ~0UL;
    return 0;
}

int sigaddset(sigset_t *set, int sig) {
    if (!set || !signal_valid(sig)) {
        errno = EINVAL;
        return -1;
    }
    set->__bits[(sig - 1) / (8 * sizeof(long))] |= sig_bit(sig);
    return 0;
}

int sigdelset(sigset_t *set, int sig) {
    if (!set || !signal_valid(sig)) {
        errno = EINVAL;
        return -1;
    }
    set->__bits[(sig - 1) / (8 * sizeof(long))] &= ~sig_bit(sig);
    return 0;
}

int sigismember(const sigset_t *set, int sig) {
    if (!set || !signal_valid(sig)) {
        errno = EINVAL;
        return -1;
    }
    return (set->__bits[(sig - 1) / (8 * sizeof(long))] & sig_bit(sig)) != 0;
}

int sigisemptyset(const sigset_t *set) {
    for (size_t i = 0; i < sizeof(set->__bits) / sizeof(set->__bits[0]); i++) {
        if (set->__bits[i])
            return 0;
    }
    return 1;
}
