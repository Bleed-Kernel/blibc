#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>

struct timeval {
    time_t tv_sec;
    long   tv_usec;
};

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval *tv, void *tz);

struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

int getitimer(int which, struct itimerval *value);
int setitimer(int which, const struct itimerval *value, struct itimerval *old);

#define timerisset(t)  ((t)->tv_sec || (t)->tv_usec)
#define timerclear(t)  ((t)->tv_sec = (t)->tv_usec = 0)
#define timercmp(a, b, op) \
    ((a)->tv_sec == (b)->tv_sec ? (a)->tv_usec op (b)->tv_usec : (a)->tv_sec op (b)->tv_sec)
#define timeradd(a, b, r) do { \
    (r)->tv_sec = (a)->tv_sec + (b)->tv_sec; (r)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
    if ((r)->tv_usec >= 1000000) { (r)->tv_sec++; (r)->tv_usec -= 1000000; } } while (0)
#define timersub(a, b, r) do { \
    (r)->tv_sec = (a)->tv_sec - (b)->tv_sec; (r)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((r)->tv_usec < 0) { (r)->tv_sec--; (r)->tv_usec += 1000000; } } while (0)
int utimes(const char *path, const struct timeval times[2]);

#ifdef __cplusplus
}
#endif

// linux has it here too :) were such shills
#include <sys/select.h>
