#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define SYSTEM_SHELL "/initrd/bin/verdict"

int system(const char *command) {
    if (!command)
        return access(SYSTEM_SHELL, X_OK) == 0;

    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        char *const argv[] = { "verdict", "-c", (char *)command, NULL };
        execv(SYSTEM_SHELL, argv);
        _exit(127);
    }

    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR)
            return -1;
    }
    return status;
}
