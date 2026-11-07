#pragma once

// what glibc and musl use to pick feature levels, blibc has everything on all the time
#if !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE) && !defined(_GNU_SOURCE) && !defined(_BSD_SOURCE)
#define _BSD_SOURCE 1
#define _XOPEN_SOURCE 700
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#define __BLIBC__ 1

// libc++ picks its bleed paths on this
#ifndef __bleed__
#define __bleed__ 1
#endif
