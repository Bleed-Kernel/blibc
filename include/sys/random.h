#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <sys/types.h>

#define GRND_NONBLOCK   0x1
#define GRND_RANDOM     0x2
#define GRND_INSECURE   0x4

// the kernels chacha20 generator, it never blocks
ssize_t getrandom(void *buf, size_t len, unsigned int flags);
int getentropy(void *buf, size_t len);

#ifdef __cplusplus
}
#endif
