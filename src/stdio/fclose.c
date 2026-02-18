#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/syscalls/close.h>

int fclose(FILE *stream) {
    if (!stream)
        return -1;

    if (stream->flags & FILE_STATIC) {
        return fflush(stream);
    }

    _close(stream->fd);
    free(stream->buffer);
    free(stream);

    return 0;
}
