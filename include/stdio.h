#pragma once
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define EOF (-1)

#define FILE_READ   0x01
#define FILE_WRITE  0x02
#define FILE_APPEND 0x04
#define FILE_STATIC 0x08

typedef struct FILE {
    int     fd;

    unsigned char *buffer;
    size_t  buf_size;
    size_t  buf_pos;
    size_t  buf_len;

    int     flags;
    int     eof;
    int     error;
} FILE;

extern FILE *const stdin;
extern FILE *const stdout;
extern FILE *const stderr;

int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int fprintf(FILE *stream, const char *fmt, ...);
int vfprintf(FILE *stream, const char *fmt, va_list ap);
int dprintf(int fd, const char *fmt, ...);
int vdprintf(int fd, const char *fmt, va_list ap);
int putchar(int c);
int puts(const char *s);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int sprintf(char *str, const char *fmt, ...);
int sscanf(const char *str, const char *fmt, ...);
int vsscanf(const char *str, const char *fmt, va_list ap);
int fscanf(FILE *stream, const char *fmt, ...);
int vfscanf(FILE *stream, const char *fmt, va_list ap);
int scanf(const char *fmt, ...);
char *fgets(char *s, int size, FILE *stream);
int fgetc(FILE *stream);
int getc(FILE *stream);
int getchar(void);
int ungetc(int c, FILE *stream);
int fputc(int c, FILE *stream);
int putc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
ssize_t getline(char **line, size_t *cap, FILE *stream);
ssize_t getdelim(char **line, size_t *cap, int delim, FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);
int fileno(FILE *stream);
void rewind(FILE *stream);
void perror(const char *s);

FILE *fopen(const char *path, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fflush(FILE *stream);
int fclose(FILE *stream);
long ftell(FILE *stream);
int fseek(FILE *stream, long offset, int whence);

//stubs for now
int remove(const char *path);
int rename(const char *oldpath, const char *newpath);

int tell(void* handle);
