#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// read and write underneath
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
#define sendfile64 sendfile

#ifdef __cplusplus
}
#endif
