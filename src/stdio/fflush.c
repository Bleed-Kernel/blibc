#include <libc/stdio.h>

int fflush(FILE *stream) {
    if (!stream)
        return 0;

    if (stream->flags & FILE_READ) {
        stream->buf_pos = 0;
        stream->buf_len = 0;
    }

    return stream->error ? -1 : 0;
}
