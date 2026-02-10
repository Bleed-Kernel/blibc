#include <stdint.h>
#include <libc/syscalls/seek.h>
#include <libc/stdio.h>

int tell(void* handle){
    int fd = (int)(uintptr_t)handle;
    long r = _seek(fd, 0, SEEK_CUR);
    return (r<0)?-1:(int)r;
}