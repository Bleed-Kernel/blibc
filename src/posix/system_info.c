#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <bleed/console.h>
#include <bleed/scheduler.h>
#include <bleed/sysinfo.h>
#include <bleed/syscalls.h>

#include "compat_errno.h"

#define HOSTNAME_PATH "/initrd/etc/hostname"
#define VERSION_PATH  "/initrd/proc/version"

static int read_line(const char *path, char *buf, size_t size) {
    int fd = _open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    long n = _read(fd, buf, size - 1);
    _close(fd);
    if (n <= 0)
        return -1;

    buf[n] = '\0';
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

int gethostname(char *name, size_t len) {
    char buf[_UTSNAME_LENGTH];
    if (!name || !len) {
        errno = EINVAL;
        return -1;
    }
    if (read_line(HOSTNAME_PATH, buf, sizeof(buf)) < 0)
        strcpy(buf, "bleed");

    strncpy(name, buf, len - 1);
    name[len - 1] = '\0';
    return 0;
}

int uname(struct utsname *buf) {
    if (!buf) {
        errno = EFAULT;
        return -1;
    }

    memset(buf, 0, sizeof(*buf));
    strcpy(buf->sysname, "Bleed");
    gethostname(buf->nodename, sizeof(buf->nodename));
    if (read_line(VERSION_PATH, buf->release, sizeof(buf->release)) < 0)
        strcpy(buf->release, "unknown");
    strcpy(buf->version, "Bleed Kernel");
    strcpy(buf->machine, "x86_64");
    return 0;
}

static struct passwd root_user;

static struct passwd *root(void) {
    const char *home  = getenv("HOME");
    const char *shell = getenv("SHELL");

    root_user.pw_name   = "root";
    root_user.pw_passwd = "";
    root_user.pw_uid    = 0;
    root_user.pw_gid    = 0;
    root_user.pw_gecos  = "";
    root_user.pw_dir    = (char *)(home ? home : "/");
    root_user.pw_shell  = (char *)(shell ? shell : "/initrd/bin/verdict");
    return &root_user;
}

struct passwd *getpwuid(uid_t uid) {
    return uid == 0 ? root() : NULL;
}

struct passwd *getpwnam(const char *name) {
    return name && strcmp(name, "root") == 0 ? root() : NULL;
}

uid_t getuid(void)  { return 0; }
uid_t geteuid(void) { return 0; }
gid_t getgid(void)  { return 0; }
gid_t getegid(void) { return 0; }

pid_t getppid(void) {
    long ret = _getppid();
    if (ret < 0)
        return (pid_t)posix_errno_from_ret(ret);
    return (pid_t)ret;
}

char *ttyname(int fd) {
    static char name[32];
    uint32_t index;

    if (_ioctl(fd, TTY_IOCTL_GET_INDEX, &index) != 0) {
        errno = ENOTTY;
        return NULL;
    }
    snprintf(name, sizeof(name), "/dev/tty%u", index);
    return name;
}

// theres no symlinks anywhere
ssize_t readlink(const char *path, char *buf, size_t size) {
    (void)path; (void)buf; (void)size;
    errno = EINVAL;
    return -1;
}

unsigned int if_nametoindex(const char *name) {
    (void)name;
    return 0;
}

char *if_indextoname(unsigned int index, char *name) {
    (void)index; (void)name;
    errno = ENXIO;
    return NULL;
}

long sysconf(int name) {
    system_memory_info_t mem;

    switch (name) {
    case _SC_CLK_TCK:
        return 100;
    case _SC_OPEN_MAX:
        return 256;
    case _SC_PAGESIZE:
        return 4096;
    // the kernel only runs on the boot cpu
    case _SC_NPROCESSORS_CONF:
    case _SC_NPROCESSORS_ONLN:
        return 1;
    case _SC_PHYS_PAGES:
        if (_meminfo(&mem) != 0)
            return -1;
        return (long)(mem.MEMORY_USABLE / 4096);
    case _SC_HOST_NAME_MAX:
        return _UTSNAME_LENGTH - 1;
    }

    errno = EINVAL;
    return -1;
}
