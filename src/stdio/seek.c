#include <libc/stdio.h>
#include <libc/syscalls/seek.h>

int fseek(FILE *stream, long offset, int whence) {
    long ret;

    if (!stream) {
        return -1;
    }

    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        stream->error = 1;
        return -1;
    }

    if (whence == SEEK_CUR && (stream->flags & FILE_READ) && stream->buf_len >= stream->buf_pos) {
        offset -= (long)(stream->buf_len - stream->buf_pos);
    }

    ret = _seek(stream->fd, offset, whence);
    if (ret < 0) {
        stream->error = 1;
        return -1;
    }

    stream->buf_pos = 0;
    stream->buf_len = 0;
    stream->eof = 0;
    return 0;
}
