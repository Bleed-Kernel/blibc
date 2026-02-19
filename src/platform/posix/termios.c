#include <libc/termios.h>

#include <libc/devices/console.h>
#include <libc/errno.h>
#include <libc/string.h>
#include <libc/syscalls/ioctl.h>

int tcgetattr(int fd, struct termios *termios_p) {
    int tty_flags = 0;

    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }

    if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &tty_flags) < 0) {
        errno = ENOTTY;
        return -1;
    }

    memset(termios_p, 0, sizeof(*termios_p));
    if (tty_flags & TTY_ECHO) termios_p->c_lflag |= ECHO;
    if (tty_flags & TTY_CANNONICAL) termios_p->c_lflag |= ICANON;

    termios_p->c_cc[VMIN] = (tty_flags & TTY_NONBLOCK) ? 0 : 1;
    termios_p->c_cc[VTIME] = 0;
    return 0;
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    int tty_flags = 0;

    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }

    if (optional_actions != TCSANOW &&
        optional_actions != TCSADRAIN &&
        optional_actions != TCSAFLUSH) {
        errno = EINVAL;
        return -1;
    }

    if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &tty_flags) < 0) {
        errno = ENOTTY;
        return -1;
    }

    if (termios_p->c_lflag & ECHO) tty_flags |= TTY_ECHO;
    else tty_flags &= ~TTY_ECHO;

    if (termios_p->c_lflag & ICANON) tty_flags |= TTY_CANNONICAL;
    else tty_flags &= ~TTY_CANNONICAL;

    if (termios_p->c_cc[VMIN] == 0 && termios_p->c_cc[VTIME] == 0) {
        tty_flags |= TTY_NONBLOCK;
    } else {
        tty_flags &= ~TTY_NONBLOCK;
    }

    if (_ioctl(fd, TTY_IOCTL_SET_FLAGS, &tty_flags) < 0) {
        errno = ENOTTY;
        return -1;
    }

    return 0;
}

void cfmakeraw(struct termios *termios_p) {
    if (!termios_p) return;

    termios_p->c_iflag = 0;
    termios_p->c_oflag = 0;
    termios_p->c_cflag = 0;
    termios_p->c_lflag = 0;
    termios_p->c_cc[VMIN] = 1;
    termios_p->c_cc[VTIME] = 0;
}
