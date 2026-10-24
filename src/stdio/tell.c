#include <stdint.h>
#include <bleed/syscalls.h>
#include <stdio.h>

long ftell(FILE *stream) {
    if (!stream) {
        return -1;
    }

    long pos = _seek(stream->fd, 0, SEEK_CUR);
    if (pos < 0) {
        return -1;
    }

    if ((stream->flags & FILE_READ) && stream->buf_len >= stream->buf_pos) {
        pos -= (long)(stream->buf_len - stream->buf_pos);
    }

    return pos;
}

int tell(void *handle) {
    FILE stream = {0};
    stream.fd = (int)(uintptr_t)handle;
    return (int)ftell(&stream);
}

off_t ftello(FILE *stream) {
    return (off_t)ftell(stream);
}
