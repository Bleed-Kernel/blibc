#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

static const char *log_ident;
static int log_opts;
static int log_mask = 0xff;

void openlog(const char *ident, int option, int facility) {
    (void)facility;
    log_ident = ident;
    log_opts = option;
}

void closelog(void) {
    log_ident = NULL;
}

int setlogmask(int mask) {
    int old = log_mask;
    if (mask)
        log_mask = mask;
    return old;
}

void vsyslog(int priority, const char *fmt, va_list ap) {
    if (!(log_mask & LOG_MASK(LOG_PRI(priority))))
        return;
    char line[1024];
    int n = 0;
    if (log_ident)
        n = snprintf(line, sizeof(line), (log_opts & LOG_PID) ? "%s[%d]: " : "%s: ", log_ident, (int)getpid());
    if (n < 0 || n >= (int)sizeof(line))
        n = 0;
    vsnprintf(line + n, sizeof(line) - n, fmt, ap);
    fprintf(stderr, "%s\n", line);
}

void syslog(int priority, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(priority, fmt, ap);
    va_end(ap);
}
