#include <libc/termios.h>

#include <libc/devices/console.h>
#include <libc/errno.h>
#include <libc/string.h>
#include <libc/syscalls/ioctl.h>

static int termios_from_legacy_flags(int fd, struct termios *termios_p) {
    int tty_flags = 0;

    if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &tty_flags) < 0)
        return -1;

    memset(termios_p, 0, sizeof(*termios_p));
    if (tty_flags & TTY_ECHO) termios_p->c_lflag |= ECHO;
    if (tty_flags & TTY_CANNONICAL) termios_p->c_lflag |= ICANON;
    termios_p->c_lflag |= ISIG;
    termios_p->c_cc[VINTR] = 3;
    termios_p->c_cc[VERASE] = 127;
    termios_p->c_cc[VMIN] = (tty_flags & TTY_NONBLOCK) ? 0 : 1;
    termios_p->c_cc[VTIME] = 0;
    return 0;
}

static int termios_to_legacy_flags(int fd, const struct termios *termios_p) {
    int tty_flags = 0;

    if (_ioctl(fd, TTY_IOCTL_GET_FLAGS, &tty_flags) < 0)
        return -1;

    if (termios_p->c_lflag & ECHO) tty_flags |= TTY_ECHO;
    else tty_flags &= ~TTY_ECHO;

    if (termios_p->c_lflag & ICANON) tty_flags |= TTY_CANNONICAL;
    else tty_flags &= ~TTY_CANNONICAL;

    if (termios_p->c_cc[VMIN] == 0 && termios_p->c_cc[VTIME] == 0)
        tty_flags |= TTY_NONBLOCK;
    else
        tty_flags &= ~TTY_NONBLOCK;

    if (_ioctl(fd, TTY_IOCTL_SET_FLAGS, &tty_flags) < 0)
        return -1;

    return 0;
}

int tcgetattr(int fd, struct termios *termios_p) {
    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }

    if (_ioctl(fd, TCGETS, termios_p) == 0)
        return 0;

    if (termios_from_legacy_flags(fd, termios_p) == 0)
        return 0;

    errno = ENOTTY;
    return -1;
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
    unsigned long request = 0;

    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }

    switch (optional_actions) {
    case TCSANOW:
        request = TCSETS;
        break;
    case TCSADRAIN:
        request = TCSETSW;
        break;
    case TCSAFLUSH:
        request = TCSETSF;
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    if (_ioctl(fd, request, (void *)termios_p) == 0)
        return 0;

    if (termios_to_legacy_flags(fd, termios_p) == 0)
        return 0;

    errno = ENOTTY;
    return -1;
}

void cfmakeraw(struct termios *termios_p) {
    if (!termios_p) return;

    termios_p->c_iflag = 0;
    termios_p->c_oflag = 0;
    termios_p->c_cflag = 0;
    termios_p->c_lflag &= ~(ECHO | ICANON | ISIG);
    termios_p->c_cc[VMIN] = 1;
    termios_p->c_cc[VTIME] = 0;
}
