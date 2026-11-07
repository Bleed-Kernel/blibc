#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define WNOHANG    1
#define WUNTRACED  2

#define WEXITSTATUS(status) (((status) >> 8) & 0xFF)
#define WTERMSIG(status)    ((status) & 0x7F)
#define WIFEXITED(status)   (WTERMSIG(status) == 0)
#define WIFSIGNALED(status) (((signed char)(((status) & 0x7f) + 1) >> 1) > 0)
#define WIFSTOPPED(status)  (((status) & 0xff) == 0x7f)
#define WSTOPSIG(status)    WEXITSTATUS(status)
#define WCOREDUMP(status)   ((status) & 0x80)
#define WIFCONTINUED(status) ((status) == 0xffff)

#define WSTOPPED   2
#define WEXITED    4
#define WCONTINUED 8
#define WNOWAIT    0x1000000

typedef enum { P_ALL = 0, P_PID = 1, P_PGID = 2, P_PIDFD = 3 } idtype_t;

pid_t waitpid(pid_t pid, int *status, int options);
pid_t wait(int *status);
struct __siginfo;
int waitid(idtype_t type, id_t id, struct __siginfo *info, int options);

#ifdef __cplusplus
}
#endif
