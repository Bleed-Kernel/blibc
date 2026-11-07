#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// rbx rbp r12 r13 r14 r15 rsp and where to come back to
typedef unsigned long jmp_buf[8];

int setjmp(jmp_buf env) __attribute__((returns_twice));
void longjmp(jmp_buf env, int val) __attribute__((noreturn));

#define _setjmp     setjmp
#define _longjmp    longjmp

#include <signal.h>

// sigsetjmp keeps the mask on the side so siglongjmp can put it back
typedef struct __sigjmp_buf {
    jmp_buf  __jb;
    int      __saved;
    sigset_t __mask;
} sigjmp_buf[1];

void __sigsetjmp_save(sigjmp_buf env, int savemask);
void siglongjmp(sigjmp_buf env, int val) __attribute__((noreturn));
#define sigsetjmp(env, savemask) (__sigsetjmp_save((env), (savemask)), setjmp((env)->__jb))

#ifdef __cplusplus
}
#endif
