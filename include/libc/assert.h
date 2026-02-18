#pragma once

#include <libc/stddef.h>

void __assert_fail(const char *expr, const char *file, int line, const char *func) __attribute__((noreturn));

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif
