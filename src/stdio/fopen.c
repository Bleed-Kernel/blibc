#include <stdio.h>
#include <stdlib.h>
#include <bleed/syscalls.h>
#include <fcntl.h>

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

// a FILE over an fd thats already open, the mode only says which way it goes
FILE *fdopen(int fd, const char *mode) {
    int open_flags = 0;
    int file_flags = 0;

    if (fd < 0 || parse_mode(mode, &open_flags, &file_flags) < 0)
        return NULL;

    FILE *f = malloc(sizeof(FILE));
    if (!f)
        return NULL;
    f->buffer = malloc(DEFAULT_BUFSIZE);
    if (!f->buffer) {
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

FILE *freopen(const char *path, const char *mode, FILE *stream) {
    int open_flags = 0;
    int file_flags = 0;

    if (!stream || parse_mode(mode, &open_flags, &file_flags) < 0)
        return NULL;

    fflush(stream);
    if (path) {
        int fd = _open(path, open_flags);
        if (fd < 0)
            return NULL;
        if (fd != stream->fd) {
            _dup2(fd, stream->fd);
            _close(fd);
        }
    }

    stream->buf_pos = 0;
    stream->buf_len = 0;
    stream->flags   = file_flags | (stream->flags & FILE_STATIC);
    stream->eof     = 0;
    stream->error   = 0;
    return stream;
}

// only unbuffered is honoured
int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
    (void)buf;
    (void)size;
    if (!stream)
        return -1;

    fflush(stream);
    if (mode == _IONBF && stream->buffer) {
        if (!(stream->flags & FILE_STATIC))
            free(stream->buffer);
        stream->buffer   = NULL;
        stream->buf_size = 0;
        stream->buf_pos  = 0;
        stream->buf_len  = 0;
    }
    return 0;
}

void setbuf(FILE *stream, char *buf) {
    setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}
