#include <libc/graphics/display.h>
#include <libc/syscalls/ioctl.h>
#include <libc/fs/file.h>
#include <libc/syscalls/open.h>
#include <libc/syscalls/close.h>

int get_fb_metadata(struct fb_info *out) {
    int fd = _open("/dev/fb0", O_RDWR);
    if (fd < 0) return -1;

    int res = _ioctl(fd, FB_IOC_GET_INFO, out);
    _close(fd);
    return res;
}