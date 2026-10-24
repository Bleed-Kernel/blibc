#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif
typedef __WINT_TYPE__ wint_t;

typedef struct {
    unsigned state;
} mbstate_t;

#define WEOF        ((wint_t)-1)
#define WCHAR_MIN   __WCHAR_MIN__
#define WCHAR_MAX   __WCHAR_MAX__
