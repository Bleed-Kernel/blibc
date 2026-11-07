#pragma once

#include <net/ethernet.h>

#ifdef __cplusplus
extern "C" {
#endif

char *ether_ntoa(const struct ether_addr *addr);
char *ether_ntoa_r(const struct ether_addr *addr, char *buf);
struct ether_addr *ether_aton(const char *s);
struct ether_addr *ether_aton_r(const char *s, struct ether_addr *addr);

#ifdef __cplusplus
}
#endif
