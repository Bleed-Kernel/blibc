#pragma once

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNS       3
#define MAXDNSRCH   6
#define MAXRESOLVSORT 10
#define RES_INIT    0x00000001
#define RES_DEBUG   0x00000002
#define RES_USEVC   0x00000008
#define RES_RECURSE 0x00000040
#define RES_DEFNAMES 0x00000080
#define RES_DNSRCH  0x00000200
#define RES_DEFAULT (RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)

struct __res_state {
    int retrans;
    int retry;
    unsigned long options;
    int nscount;
    struct sockaddr_in nsaddr_list[MAXNS];
    unsigned short id;
    char *dnsrch[MAXDNSRCH + 1];
    char defdname[256];
    unsigned long pfcode;
    unsigned ndots:4;
    unsigned nsort:4;
    unsigned ipv6_unavail:1;
    unsigned unused:23;
    void *qhook;
    void *rhook;
    int res_h_errno;
    int _vcsock;
    unsigned _flags;
    char __pad[128];
};
typedef struct __res_state *res_state;

// the resolver lives in getaddrinfo, these only say what /etc/resolv.conf would
int  res_init(void);
int  res_ninit(res_state statp);
void res_nclose(res_state statp);
int  res_query(const char *dname, int cls, int type, unsigned char *answer, int anslen);
int  res_search(const char *dname, int cls, int type, unsigned char *answer, int anslen);
int  res_nquery(res_state statp, const char *dname, int cls, int type, unsigned char *answer, int anslen);
int  res_nsearch(res_state statp, const char *dname, int cls, int type, unsigned char *answer, int anslen);

#ifdef __cplusplus
}
#endif
