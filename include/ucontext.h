#pragma once

#include <sys/ucontext.h>

#ifdef __cplusplus
extern "C" {
#endif

// getcontext and friends arent there, only the types
int  getcontext(ucontext_t *uc);
int  setcontext(const ucontext_t *uc);

#ifdef __cplusplus
}
#endif
