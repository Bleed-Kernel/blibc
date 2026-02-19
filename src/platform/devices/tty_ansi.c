#include <libc/devices/tty_ansi.h>

#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/syscalls/write.h>

static int tty_ansi_write_all(int fd, const char *buf, size_t len) {
    size_t off = 0;

    while (off < len) {
        long w = _write(fd, buf + off, (long)(len - off));
        if (w <= 0) {
            return -1;
        }
        off += (size_t)w;
    }

    return (int)len;
}

int tty_ansi_write(int fd, const char *seq) {
    if (!seq) return -1;
    return tty_ansi_write_all(fd, seq, strlen(seq));
}

int tty_ansi_csi(int fd, const char *params, char final) {
    char buf[64];
    int n;

    if (!params) params = "";
    n = snprintf(buf, sizeof(buf), "\x1b[%s%c", params, final);
    if (n < 0 || (size_t)n >= sizeof(buf)) return -1;
    return tty_ansi_write_all(fd, buf, (size_t)n);
}

int tty_ansi_reset(int fd) {
    return tty_ansi_csi(fd, "0", 'm');
}

int tty_ansi_bold(int fd, int enable) {
    return tty_ansi_csi(fd, enable ? "1" : "22", 'm');
}

int tty_ansi_underline(int fd, int enable) {
    return tty_ansi_csi(fd, enable ? "4" : "24", 'm');
}

int tty_ansi_inverse(int fd, int enable) {
    return tty_ansi_csi(fd, enable ? "7" : "27", 'm');
}

int tty_ansi_fg8(int fd, tty_ansi_color_t color, int bright) {
    char params[8];
    int base = bright ? 90 : 30;
    int code = base + (int)color;
    int n;

    if ((int)color < 0 || (int)color > 7) return -1;
    n = snprintf(params, sizeof(params), "%d", code);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'm');
}

int tty_ansi_bg8(int fd, tty_ansi_color_t color, int bright) {
    char params[8];
    int base = bright ? 100 : 40;
    int code = base + (int)color;
    int n;

    if ((int)color < 0 || (int)color > 7) return -1;
    n = snprintf(params, sizeof(params), "%d", code);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'm');
}

int tty_ansi_fg256(int fd, uint8_t index) {
    char params[16];
    int n = snprintf(params, sizeof(params), "38;5;%u", (unsigned)index);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'm');
}

int tty_ansi_bg256(int fd, uint8_t index) {
    char params[16];
    int n = snprintf(params, sizeof(params), "48;5;%u", (unsigned)index);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'm');
}

int tty_ansi_fg_rgb(int fd, uint8_t r, uint8_t g, uint8_t b) {
    char params[24];
    int n = snprintf(params, sizeof(params), "38;2;%u;%u;%u",
                     (unsigned)r, (unsigned)g, (unsigned)b);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'm');
}

int tty_ansi_bg_rgb(int fd, uint8_t r, uint8_t g, uint8_t b) {
    char params[24];
    int n = snprintf(params, sizeof(params), "48;2;%u;%u;%u",
                     (unsigned)r, (unsigned)g, (unsigned)b);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'm');
}

static int tty_ansi_csi_u32(int fd, uint32_t n, char final) {
    char params[16];
    int nbytes;

    if (n == 0) n = 1;
    nbytes = snprintf(params, sizeof(params), "%u", (unsigned)n);
    if (nbytes < 0 || (size_t)nbytes >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, final);
}

int tty_ansi_cursor_up(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'A');
}

int tty_ansi_cursor_down(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'B');
}

int tty_ansi_cursor_forward(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'C');
}

int tty_ansi_cursor_back(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'D');
}

int tty_ansi_cursor_next_line(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'E');
}

int tty_ansi_cursor_prev_line(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'F');
}

int tty_ansi_cursor_col(int fd, uint32_t col) {
    return tty_ansi_csi_u32(fd, col, 'G');
}

int tty_ansi_cursor_pos(int fd, uint32_t row, uint32_t col) {
    char params[32];
    int n;

    if (row == 0) row = 1;
    if (col == 0) col = 1;
    n = snprintf(params, sizeof(params), "%u;%u", (unsigned)row, (unsigned)col);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'H');
}

int tty_ansi_save_cursor(int fd) {
    return tty_ansi_csi(fd, "", 's');
}

int tty_ansi_restore_cursor(int fd) {
    return tty_ansi_csi(fd, "", 'u');
}

int tty_ansi_hide_cursor(int fd) {
    return tty_ansi_csi(fd, "?25", 'l');
}

int tty_ansi_show_cursor(int fd) {
    return tty_ansi_csi(fd, "?25", 'h');
}

int tty_ansi_erase_display(int fd, uint8_t mode) {
    char params[4];
    int n;

    if (mode > 2) return -1;
    n = snprintf(params, sizeof(params), "%u", (unsigned)mode);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'J');
}

int tty_ansi_erase_line(int fd, uint8_t mode) {
    char params[4];
    int n;

    if (mode > 2) return -1;
    n = snprintf(params, sizeof(params), "%u", (unsigned)mode);
    if (n < 0 || (size_t)n >= sizeof(params)) return -1;
    return tty_ansi_csi(fd, params, 'K');
}

int tty_ansi_clear_screen(int fd) {
    return tty_ansi_erase_display(fd, 2);
}

int tty_ansi_clear_line(int fd) {
    return tty_ansi_erase_line(fd, 2);
}

int tty_ansi_scroll_up(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'S');
}

int tty_ansi_scroll_down(int fd, uint32_t n) {
    return tty_ansi_csi_u32(fd, n, 'T');
}
