#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NS_PACKETSZ  512
#define NS_MAXDNAME  1025
#define NS_MAXCDNAME 255
#define NS_HFIXEDSZ  12
#define NS_QFIXEDSZ  4
#define NS_RRFIXEDSZ 10
#define NS_INT16SZ   2
#define NS_INT32SZ   4
#define PACKETSZ  NS_PACKETSZ
#define MAXDNAME  NS_MAXDNAME
#define HFIXEDSZ  NS_HFIXEDSZ
#define QFIXEDSZ  NS_QFIXEDSZ

typedef enum __ns_class {
    ns_c_invalid = 0, ns_c_in = 1, ns_c_2 = 2, ns_c_chaos = 3, ns_c_hs = 4,
    ns_c_none = 254, ns_c_any = 255, ns_c_max = 65536
} ns_class;

typedef enum __ns_type {
    ns_t_invalid = 0, ns_t_a = 1, ns_t_ns = 2, ns_t_cname = 5, ns_t_soa = 6, ns_t_ptr = 12,
    ns_t_mx = 15, ns_t_txt = 16, ns_t_aaaa = 28, ns_t_srv = 33, ns_t_opt = 41,
    ns_t_svcb = 64, ns_t_https = 65, ns_t_any = 255, ns_t_max = 65536
} ns_type;

#define C_IN    ns_c_in
#define T_A     ns_t_a
#define T_AAAA  ns_t_aaaa
#define T_CNAME ns_t_cname
#define T_TXT   ns_t_txt
#define T_SRV   ns_t_srv

#ifdef __cplusplus
}
#endif
