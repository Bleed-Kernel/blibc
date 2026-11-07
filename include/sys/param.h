#pragma once

#include <limits.h>
#include <endian.h>

#define MAXSYMLINKS 20
#define MAXHOSTNAMELEN 64
#define MAXNAMLEN 255
#define MAXPATHLEN PATH_MAX
#define NBBY 8
#define NGROUPS NGROUPS_MAX
#define NOGROUP (-1)
#define NOFILE 256
#define HZ 100
#define DEV_BSIZE 512
#define EXEC_PAGESIZE 4096

#ifndef NGROUPS_MAX
#define NGROUPS_MAX 32
#endif

#define setbit(x, i)  ((unsigned char *)(x))[(i) / NBBY] |= 1 << ((i) % NBBY)
#define clrbit(x, i)  ((unsigned char *)(x))[(i) / NBBY] &= ~(1 << ((i) % NBBY))
#define isset(x, i)   (((const unsigned char *)(x))[(i) / NBBY] & (1 << ((i) % NBBY)))
#define isclr(x, i)   !isset(x, i)

#define howmany(n, d) (((n) + ((d) - 1)) / (d))
#define roundup(n, d) (howmany(n, d) * (d))
#define powerof2(n)   !(((n) - 1) & (n))

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
