#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <netinet/in.h>
#include <sys/socket.h>

// ipv4 only for now
in_addr_t inet_addr(const char *cp);
int inet_aton(const char *cp, struct in_addr *addr);
char *inet_ntoa(struct in_addr in);
int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

#ifdef __cplusplus
}
#endif
