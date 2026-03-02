#pragma once

#include <libc/types.h>

#define WNOHANG    1
#define WUNTRACED  2

#define WEXITSTATUS(status) (((status) >> 8) & 0xFF)
#define WTERMSIG(status)    ((status) & 0x7F)
#define WIFEXITED(status)   (WTERMSIG(status) == 0)
#define WIFSIGNALED(status) (WTERMSIG(status) != 0)

pid_t waitpid(pid_t pid, int *status, int options);
pid_t wait(int *status);
