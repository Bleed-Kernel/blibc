#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/syscalls/read.h>
#include <libc/syscalls/close.h>
#include <libc/fs/file.h>

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!stream || !(stream->flags & FILE_READ))
        return 0;

    size_t total = size * nmemb;
    size_t copied = 0;
    unsigned char *out = ptr;

    while (copied < total) {

        if (stream->buf_pos >= stream->buf_len) {
            long r = _read(stream->fd, stream->buffer, stream->buf_size);
            if (r <= 0) {
                if (r == 0) stream->eof = 1;
                else stream->error = 1;
                break;
            }

            stream->buf_len = r;
            stream->buf_pos = 0;
        }

        size_t avail = stream->buf_len - stream->buf_pos;
        size_t needed = total - copied;
        size_t chunk = avail < needed ? avail : needed;

        memcpy(out + copied, stream->buffer + stream->buf_pos, chunk);

        stream->buf_pos += chunk;
        copied += chunk;
    }

    return copied / size;
}