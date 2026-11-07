#include <unistd.h>
#include <time.h>
#include <errno.h>

unsigned int sleep(unsigned int seconds) {
    struct timespec ts = { (time_t)seconds, 0 };
    return nanosleep(&ts, NULL) == 0 ? 0 : seconds;
}

int usleep(unsigned long usec) {
    struct timespec ts = { (time_t)(usec / 1000000), (long)(usec % 1000000) * 1000 };
    return nanosleep(&ts, NULL);
}

int clock_nanosleep(clockid_t clock, int flags, const struct timespec *req, struct timespec *rem) {
    struct timespec ts = *req;
    if (flags & TIMER_ABSTIME) {
        struct timespec now;
        if (clock_gettime(clock, &now) < 0)
            return EINVAL;
        ts.tv_sec  = req->tv_sec - now.tv_sec;
        ts.tv_nsec = req->tv_nsec - now.tv_nsec;
        if (ts.tv_nsec < 0) {
            ts.tv_sec--;
            ts.tv_nsec += 1000000000;
        }
        if (ts.tv_sec < 0)
            return 0;
        rem = NULL;
    }
    int saved = errno;
    if (nanosleep(&ts, rem) < 0) {
        int err = errno;
        errno = saved;
        return err;
    }
    return 0;
}
