#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/syscalls/write.h>
#include <libc/syscalls/close.h>
#include <libc/fs/file.h>

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (!stream || !(stream->flags & FILE_WRITE))
        return 0;

    size_t total = size * nmemb;
    size_t written = 0;
    const unsigned char *in = ptr;

    while (written < total) {
        long r = _write(stream->fd, in + written, total - written);
        if (r <= 0) {
            stream->error = 1;
            break;
        }
        written += r;
    }

    return written / size;
}