#pragma once
#include <stdint.h>

#define TTY_ECHO        (1 << 1)
#define TTY_CANNONICAL  (1 << 2)
#define TTY_NONBLOCK    (1 << 4)

// LEGACY IOCTLS
#define TTY_IOCTL_GET_FLAGS         0x5401
#define TTY_IOCTL_SET_FLAGS         0x5402
#define TTY_IOCTL_GET_CURSOR        0x5403
#define TTY_IOCTL_SET_CURSOR        0x5404
#define TTY_IOCTL_GET_WINSIZE       0x5405
#define TTY_IOCTL_GET_INDEX         0x5406

// termios
#define TTY_IOCTL_TCGETS            0x5407
#define TTY_IOCTL_TCSETS            0x5408
#define TTY_IOCTL_TCSETSW           0x5409
#define TTY_IOCTL_TCSETSF           0x540A
#define TTY_IOCTL_TIOCGWINSZ        0x5413
#define TTY_IOCTL_TIOCSWINSZ        0x5414
#define TTY_IOCTL_FIONBIO           0x5421

// TTY 
#define TTY_IOCTL_CREATE            0x5422
#define TTY_IOCTL_SET_ACTIVE        0x5423
#define TTY_IOCTL_GET_ACTIVE_INDEX  0x5424

// Window 
#define TTY_IOCTL_WM_CLAIM          0x5425
#define TTY_IOCTL_WM_RELEASE        0x5426
#define TTY_IOCTL_WM_GET_MODE       0x5427
#define TTY_IOCTL_WM_SET_MODE       0x5428
#define TTY_IOCTL_WM_GET_OWNER      0x5429

#define TTY_WM_MODE_LEGACY          0u
#define TTY_WM_MODE_EXTERNAL        1u

typedef struct {
    uint32_t x;
    uint32_t y;
} tty_cursor_t;

typedef struct {
    uint32_t cols;
    uint32_t rows;
} tty_winsize_t;
