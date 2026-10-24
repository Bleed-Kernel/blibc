#pragma once

#ifdef __cplusplus
#define NULL __null
#else
#define NULL ((void*)0)
#endif

#ifndef offsetof
# define offsetof(type, field) __builtin_offsetof(type, field)
#endif