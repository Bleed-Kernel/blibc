#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/random.h>
#include <bleed/syscalls.h>
#include "../posix/compat_errno.h"

#define ARC4_BUF 256

static uint8_t arc4_buf[ARC4_BUF];
static size_t  arc4_left;
static pid_t   arc4_pid;

ssize_t getrandom(void *buf, size_t len, unsigned int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_GETRANDOM), "D"(buf), "S"(len), "d"((long)flags)
        : "rcx", "r11", "memory"
    );
    return ret < 0 ? posix_errno_from_ret(ret) : ret;
}

// the openbsd one, 256 bytes at most and either all of it or an error
int getentropy(void *buf, size_t len) {
    if (len > 256) {
        errno = EIO;
        return -1;
    }
    uint8_t *p = buf;
    while (len) {
        ssize_t n = getrandom(p, len, 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p   += n;
        len -= (size_t)n;
    }
    return 0;
}

// a fork child would hand out the same bytes as its parent, so a new pid starts over
static void arc4_take(void *out, size_t len) {
    pid_t now = getpid();
    if (now != arc4_pid) {
        arc4_pid  = now;
        arc4_left = 0;
    }

    uint8_t *p = out;
    while (len) {
        if (!arc4_left) {
            if (getentropy(arc4_buf, sizeof(arc4_buf)) != 0)
                abort();
            arc4_left = sizeof(arc4_buf);
        }
        size_t n = len < arc4_left ? len : arc4_left;
        uint8_t *from = arc4_buf + sizeof(arc4_buf) - arc4_left;
        memcpy(p, from, n);
        memset(from, 0, n);
        arc4_left -= n;
        p         += n;
        len       -= n;
    }
}

uint32_t arc4random(void) {
    uint32_t v;
    arc4_take(&v, sizeof(v));
    return v;
}

void arc4random_buf(void *buf, size_t len) {
    arc4_take(buf, len);
}

// rejection sampling, plain % would lean towards the small numbers
uint32_t arc4random_uniform(uint32_t upper) {
    if (upper < 2)
        return 0;
    uint32_t min = (uint32_t)-upper % upper;
    for (;;) {
        uint32_t r = arc4random();
        if (r >= min)
            return r % upper;
    }
}
