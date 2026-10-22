#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <bleed/syscalls.h>
#include <bleed/hpet.h>

static int hpet_fd = -2;

// femtoseconds since boot, 0 if theres no hpet
static uint64_t hpet_now(void) {
    uint64_t now = 0;

    if (hpet_fd == -2)
        hpet_fd = _open("/dev/hpet", O_RDONLY);
    if (hpet_fd < 0)
        return 0;

    if (_ioctl(hpet_fd, HPET_IOCTL_GET_FEMTOSECONDS, &now) == 0)
        return now;
    if (_read(hpet_fd, &now, sizeof(now)) == (long)sizeof(now))
        return now;
    return 0;
}

// days since 1970-01-01, howard hinnants civil calendar maths
static long days_from_civil(long y, int m, int d) {
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = y - era * 400;
    long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static void civil_from_days(long z, long *y, int *m, int *d) {
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    long doe = z - era * 146097;
    long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    long mp  = (5 * doy + 2) / 153;
    *d = (int)(doy - (153 * mp + 2) / 5 + 1);
    *m = (int)(mp < 10 ? mp + 3 : mp - 9);
    *y = yoe + era * 400 + (*m <= 2);
}

static time_t rtc_seconds(void) {
    rtc_time_t rtc;
    if (_time(&rtc) < 0)
        return 0;
    return (time_t)days_from_civil(rtc.year, rtc.mon, rtc.day) * 86400
         + rtc.hour * 3600 + rtc.min * 60 + rtc.sec;
}

// the rtc only counts whole seconds, so real time is read off it once and then carried on by
// the hpet, otherwise it would stand still for a second at a time
static int      wall_read;
static time_t   wall_base;
static uint64_t wall_base_fs;

static void wall_now(struct timespec *ts) {
    uint64_t fs = hpet_now();
    if (!wall_read) {
        wall_read    = 1;
        wall_base    = rtc_seconds();
        wall_base_fs = fs;
    }
    uint64_t since = fs - wall_base_fs;
    ts->tv_sec  = wall_base + (time_t)(since / femtosecondsPerSecond);
    ts->tv_nsec = (long)((since % femtosecondsPerSecond) / 1000000);
}

time_t time(time_t *tloc) {
    struct timespec ts;
    wall_now(&ts);
    if (tloc)
        *tloc = ts.tv_sec;
    return ts.tv_sec;
}

int clock_gettime(clockid_t id, struct timespec *ts) {
    if (!ts) {
        errno = EINVAL;
        return -1;
    }
    if (id == CLOCK_REALTIME) {
        wall_now(ts);
        return 0;
    }
    if (id == CLOCK_MONOTONIC) {
        uint64_t fs = hpet_now();
        ts->tv_sec  = (time_t)(fs / femtosecondsPerSecond);
        ts->tv_nsec = (long)((fs % femtosecondsPerSecond) / 1000000);
        return 0;
    }
    errno = EINVAL;
    return -1;
}

int gettimeofday(struct timeval *tv, void *tz) {
    (void)tz;
    if (tv) {
        struct timespec ts;
        wall_now(&ts);
        tv->tv_sec  = ts.tv_sec;
        tv->tv_usec = ts.tv_nsec / 1000;
    }
    return 0;
}

// theres no per task cpu time to hand out, so its time since the program first asked
clock_t clock(void) {
    static uint64_t start;
    uint64_t fs = hpet_now();
    if (!start)
        start = fs;
    return (clock_t)((fs - start) / femtosecondsPerMicrosecond);
}

double difftime(time_t end, time_t start) {
    return (double)(end - start);
}

struct tm *gmtime_r(const time_t *timer, struct tm *out) {
    if (!timer || !out)
        return NULL;

    time_t t    = *timer;
    long   days = (long)(t / 86400);
    long   secs = (long)(t % 86400);
    if (secs < 0) {
        secs += 86400;
        days--;
    }

    long y;
    int  m, d;
    civil_from_days(days, &y, &m, &d);

    out->tm_sec   = (int)(secs % 60);
    out->tm_min   = (int)(secs / 60 % 60);
    out->tm_hour  = (int)(secs / 3600);
    out->tm_mday  = d;
    out->tm_mon   = m - 1;
    out->tm_year  = (int)(y - 1900);
    out->tm_wday  = (int)((days % 7 + 11) % 7);    // 1970-01-01 was a thursday
    out->tm_yday  = (int)(days - days_from_civil(y, 1, 1));
    out->tm_isdst = 0;
    return out;
}

struct tm *gmtime(const time_t *timer) {
    static struct tm out;
    return gmtime_r(timer, &out);
}

struct tm *localtime_r(const time_t *timer, struct tm *out) {
    return gmtime_r(timer, out);
}

struct tm *localtime(const time_t *timer) {
    return gmtime(timer);
}

// fields out of range are folded back in, so adding 40 to tm_mday moves into the next month
time_t mktime(struct tm *tm) {
    if (!tm)
        return (time_t)-1;

    long mon  = tm->tm_mon;
    long year = tm->tm_year + 1900L + mon / 12;
    mon %= 12;
    if (mon < 0) {
        mon += 12;
        year--;
    }

    time_t t = (time_t)days_from_civil(year, (int)mon + 1, 1) * 86400
             + (time_t)(tm->tm_mday - 1) * 86400
             + (time_t)tm->tm_hour * 3600 + (time_t)tm->tm_min * 60 + tm->tm_sec;
    gmtime_r(&t, tm);
    return t;
}

static const char *day_names[]   = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char *month_names[] = { "January", "February", "March", "April", "May", "June", "July",
                                     "August", "September", "October", "November", "December" };

typedef struct {
    char  *s;
    size_t max, n;
} out_t;

static void put(out_t *o, const char *text, size_t len) {
    for (size_t i = 0; i < len; i++, o->n++) {
        if (o->n < o->max)
            o->s[o->n] = text[i];
    }
}

static void put_str(out_t *o, const char *text) {
    put(o, text, strlen(text));
}

static void put_num(out_t *o, long v, int width, char pad) {
    char buf[24];
    int  n = 0, neg = v < 0;
    unsigned long u = neg ? (unsigned long)-v : (unsigned long)v;
    do {
        buf[n++] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    while (n < width)
        buf[n++] = pad;
    if (neg)
        buf[n++] = '-';
    for (int i = n - 1; i >= 0; i--)
        put(o, &buf[i], 1);
}

static void format(out_t *o, const char *fmt, const struct tm *tm) {
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            put(o, fmt, 1);
            continue;
        }

        int wday = tm->tm_wday % 7, mon = tm->tm_mon % 12;
        if (wday < 0) wday += 7;
        if (mon < 0) mon += 12;
        int hour12 = tm->tm_hour % 12 ? tm->tm_hour % 12 : 12;

        switch (*++fmt) {
        case 'a': put(o, day_names[wday], 3); break;
        case 'A': put_str(o, day_names[wday]); break;
        case 'b':
        case 'h': put(o, month_names[mon], 3); break;
        case 'B': put_str(o, month_names[mon]); break;
        case 'c': format(o, "%a %b %e %H:%M:%S %Y", tm); break;
        case 'C': put_num(o, (tm->tm_year + 1900L) / 100, 2, '0'); break;
        case 'd': put_num(o, tm->tm_mday, 2, '0'); break;
        case 'D': format(o, "%m/%d/%y", tm); break;
        case 'e': put_num(o, tm->tm_mday, 2, ' '); break;
        case 'F': format(o, "%Y-%m-%d", tm); break;
        case 'H': put_num(o, tm->tm_hour, 2, '0'); break;
        case 'I': put_num(o, hour12, 2, '0'); break;
        case 'j': put_num(o, tm->tm_yday + 1, 3, '0'); break;
        case 'm': put_num(o, tm->tm_mon + 1, 2, '0'); break;
        case 'M': put_num(o, tm->tm_min, 2, '0'); break;
        case 'n': put(o, "\n", 1); break;
        case 'p': put_str(o, tm->tm_hour < 12 ? "AM" : "PM"); break;
        case 'r': format(o, "%I:%M:%S %p", tm); break;
        case 'R': format(o, "%H:%M", tm); break;
        case 's': {
            struct tm copy = *tm;
            put_num(o, (long)mktime(&copy), 0, '0');
            break;
        }
        case 'S': put_num(o, tm->tm_sec, 2, '0'); break;
        case 't': put(o, "\t", 1); break;
        case 'T': format(o, "%H:%M:%S", tm); break;
        case 'u': put_num(o, wday ? wday : 7, 0, '0'); break;
        case 'w': put_num(o, wday, 0, '0'); break;
        case 'x': format(o, "%m/%d/%y", tm); break;
        case 'X': format(o, "%H:%M:%S", tm); break;
        case 'y': put_num(o, (tm->tm_year + 1900L) % 100, 2, '0'); break;
        case 'Y': put_num(o, tm->tm_year + 1900L, 0, '0'); break;
        case 'z': put_str(o, "+0000"); break;
        case 'Z': put_str(o, "UTC"); break;
        case '%': put(o, "%", 1); break;
        case '\0': return;
        default:  put(o, fmt - 1, 2); break;
        }
    }
}

// 0 if it didnt fit, the same as everyone else
size_t strftime(char *s, size_t max, const char *fmt, const struct tm *tm) {
    if (!s || !fmt || !tm || max == 0)
        return 0;
    out_t o = { s, max, 0 };
    format(&o, fmt, tm);
    if (o.n >= max) {
        s[max - 1] = '\0';
        return 0;
    }
    s[o.n] = '\0';
    return o.n;
}

char *asctime(const struct tm *tm) {
    static char buf[32];
    strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Y\n", tm);
    return buf;
}

char *ctime(const time_t *timer) {
    return asctime(localtime(timer));
}

// the kernel sleeps in whole milliseconds, anything shorter still gets one
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!req || req->tv_nsec < 0 || req->tv_nsec >= 1000000000L || req->tv_sec < 0) {
        errno = EINVAL;
        return -1;
    }

    long ms = (long)req->tv_sec * 1000 + (req->tv_nsec + 999999) / 1000000;
    long r  = _poll(NULL, 0, ms);
    if (rem) {
        rem->tv_sec  = 0;
        rem->tv_nsec = 0;
    }
    if (r < 0) {
        errno = (int)-r;
        return -1;
    }
    return 0;
}
