#include <libc/stdio.h>
#include <libc/unistd.h>

static unsigned char stdin_buffer[256];

static FILE stdin_stream = {
    .fd = STDIN_FILENO,
    .buffer = stdin_buffer,
    .buf_size = sizeof(stdin_buffer),
    .buf_pos = 0,
    .buf_len = 0,
    .flags = FILE_READ | FILE_STATIC,
    .eof = 0,
    .error = 0,
};

static FILE stdout_stream = {
    .fd = STDOUT_FILENO,
    .buffer = NULL,
    .buf_size = 0,
    .buf_pos = 0,
    .buf_len = 0,
    .flags = FILE_WRITE | FILE_STATIC,
    .eof = 0,
    .error = 0,
};

static FILE stderr_stream = {
    .fd = STDERR_FILENO,
    .buffer = NULL,
    .buf_size = 0,
    .buf_pos = 0,
    .buf_len = 0,
    .flags = FILE_WRITE | FILE_STATIC,
    .eof = 0,
    .error = 0,
};

FILE *const stdin = &stdin_stream;
FILE *const stdout = &stdout_stream;
FILE *const stderr = &stderr_stream;
