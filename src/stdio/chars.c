#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int fgetc(FILE *stream) {
    if (stream && stream->buf_pos < stream->buf_len)
        return stream->buffer[stream->buf_pos++];

    unsigned char ch;
    return fread(&ch, 1, 1, stream) == 1 ? ch : EOF;
}

int getc(FILE *stream) {
    return fgetc(stream);
}

int getchar(void) {
    return fgetc(stdin);
}

int ungetc(int c, FILE *stream) {
    if (c == EOF || !stream || !(stream->flags & FILE_READ) || !stream->buffer)
        return EOF;

    if (stream->buf_pos > 0) {
        stream->buffer[--stream->buf_pos] = (unsigned char)c;
    } else if (stream->buf_len < stream->buf_size) {
        memmove(stream->buffer + 1, stream->buffer, stream->buf_len);
        stream->buffer[0] = (unsigned char)c;
        stream->buf_len++;
    } else {
        return EOF;
    }
    stream->eof = 0;
    return (unsigned char)c;
}

int fputc(int c, FILE *stream) {
    unsigned char ch = (unsigned char)c;
    return fwrite(&ch, 1, 1, stream) == 1 ? ch : EOF;
}

int putc(int c, FILE *stream) {
    return fputc(c, stream);
}

int fputs(const char *s, FILE *stream) {
    if (!s)
        return EOF;
    size_t len = strlen(s);
    return fwrite(s, 1, len, stream) == len ? 0 : EOF;
}

ssize_t getdelim(char **line, size_t *cap, int delim, FILE *stream) {
    if (!line || !cap || !stream) {
        errno = EINVAL;
        return -1;
    }

    size_t len = 0;
    int    c;
    while ((c = fgetc(stream)) != EOF) {
        if (len + 2 > *cap || !*line) {
            size_t grown = *cap < 64 ? 128 : *cap * 2;
            char  *more  = realloc(*line, grown);
            if (!more) {
                errno = ENOMEM;
                return -1;
            }
            *line = more;
            *cap  = grown;
        }
        (*line)[len++] = (char)c;
        if (c == delim)
            break;
    }

    if (len == 0)
        return -1;
    (*line)[len] = '\0';
    return (ssize_t)len;
}

ssize_t getline(char **line, size_t *cap, FILE *stream) {
    return getdelim(line, cap, '\n', stream);
}

int feof(FILE *stream) {
    return stream ? stream->eof : 0;
}

int ferror(FILE *stream) {
    return stream ? stream->error : 0;
}

void clearerr(FILE *stream) {
    if (stream) {
        stream->eof   = 0;
        stream->error = 0;
    }
}

int fileno(FILE *stream) {
    return stream ? stream->fd : -1;
}

void rewind(FILE *stream) {
    if (fseek(stream, 0, SEEK_SET) == 0)
        clearerr(stream);
}
