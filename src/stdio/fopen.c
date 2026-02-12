#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/syscalls/open.h>
#include <libc/syscalls/close.h>
#include <libc/fs/file.h>

#define DEFAULT_BUFSIZE 4096

static int parse_mode(const char *mode, int *open_flags, int *file_flags) {
    if (!mode) return -1;

    switch (mode[0]) {
        case 'r':
            *open_flags = O_RDONLY;
            *file_flags = FILE_READ;
            break;

        case 'w':
            *open_flags = O_WRONLY | O_CREAT | O_TRUNC;
            *file_flags = FILE_WRITE;
            break;

        case 'a':
            *open_flags = O_WRONLY | O_CREAT | O_APPEND;
            *file_flags = FILE_WRITE | FILE_APPEND;
            break;

        default:
            return -1;
    }

    if (mode[1] == '+') {
        *open_flags &= ~(O_RDONLY | O_WRONLY);
        *open_flags |= O_RDWR;
        *file_flags |= FILE_READ | FILE_WRITE;
    }

    return 0;
}

FILE *fopen(const char *path, const char *mode) {
    int open_flags = 0;
    int file_flags = 0;

    if (parse_mode(mode, &open_flags, &file_flags) < 0)
        return NULL;

    int fd = _open(path, open_flags);
    if (fd < 0)
        return NULL;

    FILE *f = malloc(sizeof(FILE));
    if (!f) {
        _close(fd);
        return NULL;
    }

    f->buffer = malloc(DEFAULT_BUFSIZE);
    if (!f->buffer) {
        _close(fd);
        free(f);
        return NULL;
    }

    f->fd       = fd;
    f->buf_size = DEFAULT_BUFSIZE;
    f->buf_pos  = 0;
    f->buf_len  = 0;
    f->flags    = file_flags;
    f->eof      = 0;
    f->error    = 0;

    return f;
}
