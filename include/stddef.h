#pragma once

typedef __SIZE_TYPE__    size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#ifndef __cplusplus
typedef __WCHAR_TYPE__   wchar_t;
#endif
typedef struct { long long ll __attribute__((aligned(__alignof__(long long)))); long double ld __attribute__((aligned(__alignof__(long double)))); } max_align_t;

#ifdef __cplusplus
#define NULL __null
#else
#define NULL ((void*)0)
#endif

#ifndef offsetof
# define offsetof(type, field) __builtin_offsetof(type, field)
#endif