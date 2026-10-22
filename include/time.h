#pragma once
#include <stddef.h>
#include <sys/types.h>

#define CLOCKS_PER_SEC  1000000L

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

typedef long clock_t;
typedef int  clockid_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

struct timespec {
    time_t tv_sec;
    long   tv_nsec;
};

// theres no timezones, the rtc is taken as utc and local time is the same thing
time_t time(time_t *tloc);
clock_t clock(void);
double difftime(time_t end, time_t start);
time_t mktime(struct tm *tm);
struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *timer, struct tm *out);
struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *timer, struct tm *out);
size_t strftime(char *s, size_t max, const char *fmt, const struct tm *tm);
char *asctime(const struct tm *tm);
char *ctime(const time_t *timer);

int clock_gettime(clockid_t id, struct timespec *ts);
int nanosleep(const struct timespec *req, struct timespec *rem);
