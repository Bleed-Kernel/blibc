#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/socket.h>

struct sockaddr_un {
    sa_family_t sun_family;
    char        sun_path[108];
};

#define SUN_LEN(su) (sizeof(sa_family_t) + __builtin_strlen((su)->sun_path))

#ifdef __cplusplus
}
#endif
