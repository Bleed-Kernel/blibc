#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/syscalls/close.h>

int fclose(FILE *stream) {
    if (!stream)
        return -1;

    _close(stream->fd);
    free(stream->buffer);
    free(stream);

    return 0;
}
