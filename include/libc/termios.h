#pragma once

#include <libc/types.h>
#include <libc/devices/console.h>

typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;

enum {
    VINTR = 0,
    VQUIT,
    VERASE,
    VKILL,
    VEOF,
    VTIME,
    VMIN,
    NCCS
};

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t c_cc[NCCS];
};

struct winsize {
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

#define ECHO   (1u << 0)
#define ICANON (1u << 1)
#define ISIG   (1u << 2)

#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

#define TCGETS    TTY_IOCTL_TCGETS
#define TCSETS    TTY_IOCTL_TCSETS
#define TCSETSW   TTY_IOCTL_TCSETSW
#define TCSETSF   TTY_IOCTL_TCSETSF

#define TIOCGWINSZ TTY_IOCTL_TIOCGWINSZ
#define TIOCSWINSZ TTY_IOCTL_TIOCSWINSZ

int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
void cfmakeraw(struct termios *termios_p);
