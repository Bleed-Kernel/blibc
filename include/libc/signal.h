#pragma once

#include <libc/types.h>
#include <libc/syscalls/signals.h>

typedef void (*sighandler_t)(int);

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
