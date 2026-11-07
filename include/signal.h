#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>

// linux numbering and musls layouts we only acc look at the first 64
#define NSIG  65
#define _NSIG NSIG

typedef struct {
    unsigned long __bits[128 / sizeof(long)];
} sigset_t;

typedef enum {
    SIGHUP  = 1,
    SIGINT  = 2,
    SIGQUIT = 3,
    SIGILL  = 4,
    SIGTRAP = 5,
    SIGABRT = 6,
    SIGBUS  = 7,
    SIGFPE  = 8,
    SIGKILL = 9,
    SIGUSR1 = 10,
    SIGSEGV = 11,
    SIGUSR2 = 12,
    SIGPIPE = 13,
    SIGALRM = 14,
    SIGTERM = 15,
    SIGCHLD = 17,
    SIGCONT = 18,
    SIGSTOP = 19,
    SIGTSTP = 20,
    SIGTTIN = 21,
    SIGTTOU = 22,
    SIGSTKFLT = 16,
    SIGURG  = 23,
    SIGXCPU = 24,
    SIGXFSZ = 25,
    SIGVTALRM = 26,
    SIGPROF = 27,
    SIGWINCH = 28,
    SIGIO   = 29,
    SIGPWR  = 30,
    SIGSYS  = 31
} signal_number_t;

#define SIGPOLL SIGIO
#define SIGIOT  SIGABRT
#define SIGUNUSED SIGSYS
#define SIGRTMIN 34
#define SIGRTMAX 64

typedef void (*sighandler_t)(int);
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)

#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

#define SA_NOCLDSTOP 0x1
#define SA_NOCLDWAIT 0x2
#define SA_SIGINFO   0x4
#define SA_RESTORER  0x04000000
#define SA_ONSTACK   0x08000000
#define SA_RESTART   0x10000000
#define SA_NODEFER   0x40000000
#define SA_RESETHAND 0x80000000
#define SA_NOMASK    SA_NODEFER
#define SA_ONESHOT   SA_RESETHAND

union sigval {
    int   sival_int;
    void *sival_ptr;
};

// musls siginfo_t, the kernel fills signo, code and addr
typedef struct __siginfo {
    int si_signo;
    int si_errno;
    int si_code;
    union {
        char __pad[128 - 2 * sizeof(int) - sizeof(long)];
        struct {
            union {
                struct {
                    pid_t si_pid;
                    uid_t si_uid;
                } __piduid;
                struct {
                    int si_timerid;
                    int si_overrun;
                } __timer;
            } __first;
            union {
                union sigval si_value;
                struct {
                    int si_status;
                    long si_utime, si_stime;
                } __sigchld;
            } __second;
        } __si_common;
        struct {
            void *si_addr;
            short si_addr_lsb;
        } __sigfault;
        struct {
            long si_band;
            int  si_fd;
        } __sigpoll;
    } __si_fields;
} siginfo_t;

#define si_pid    __si_fields.__si_common.__first.__piduid.si_pid
#define si_uid    __si_fields.__si_common.__first.__piduid.si_uid
#define si_status __si_fields.__si_common.__second.__sigchld.si_status
#define si_utime  __si_fields.__si_common.__second.__sigchld.si_utime
#define si_stime  __si_fields.__si_common.__second.__sigchld.si_stime
#define si_value  __si_fields.__si_common.__second.si_value
#define si_addr   __si_fields.__sigfault.si_addr
#define si_band   __si_fields.__sigpoll.si_band
#define si_fd     __si_fields.__sigpoll.si_fd

#define SI_USER     0
#define SI_KERNEL   0x80
#define SI_QUEUE    (-1)
#define SI_TKILL    (-6)
#define SEGV_MAPERR 1
#define SEGV_ACCERR 2
#define BUS_ADRALN  1
#define BUS_ADRERR  2
#define BUS_OBJERR  3
#define ILL_ILLOPC  1
#define ILL_ILLOPN  2
#define ILL_ILLADR  3
#define ILL_ILLTRP  4
#define ILL_PRVOPC  5
#define ILL_PRVREG  6
#define ILL_COPROC  7
#define ILL_BADSTK  8
#define FPE_INTDIV  1
#define FPE_INTOVF  2
#define FPE_FLTDIV  3
#define FPE_FLTOVF  4
#define FPE_FLTUND  5
#define FPE_FLTRES  6
#define FPE_FLTINV  7
#define FPE_FLTSUB  8
#define TRAP_BRKPT  1
#define TRAP_TRACE  2
#define POLL_IN     1
#define POLL_OUT    2
#define CLD_EXITED  1
#define CLD_KILLED  2
#define CLD_DUMPED  3
#define CLD_TRAPPED 4
#define CLD_STOPPED 5
#define CLD_CONTINUED 6

// musls struct sigaction, with blibcs old field names still there
struct sigaction {
    union {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, siginfo_t *, void *);
        uintptr_t handler;
    };
    union {
        sigset_t sa_mask;
        sigset_t mask;
    };
    union {
        int sa_flags;
        int flags;
    };
    union {
        void (*sa_restorer)(void);
        uintptr_t restorer;
    };
};

// what the kernel takes
typedef struct __ksigaction {
    uintptr_t     handler;
    unsigned long mask;
    unsigned long flags;
    uintptr_t     restorer;
} sigaction_t;

typedef struct {
    void  *ss_sp;
    int    ss_flags;
    size_t ss_size;
} stack_t;

#define SS_ONSTACK  1
#define SS_DISABLE  2
#define MINSIGSTKSZ 2048
#define SIGSTKSZ    8192

typedef int sig_atomic_t; 

#define SIG_ERR ((sighandler_t)-1)

int kill(pid_t pid, int sig);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
sighandler_t signal(int sig, sighandler_t handler);
int raise(int sig);

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int sig);
int sigdelset(sigset_t *set, int sig);
int sigismember(const sigset_t *set, int sig);
int sigisemptyset(const sigset_t *set);
int sigaltstack(const stack_t *ss, stack_t *old);
int sigsuspend(const sigset_t *mask);
int sigpending(sigset_t *set);
int siginterrupt(int sig, int flag);


struct timespec;
// waits for one of set, which the caller has blocked, and takes it without running a handler
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);

#ifdef __cplusplus
}
#endif

#include <sys/ucontext.h>
