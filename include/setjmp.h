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

#ifdef __cplusplus
}
#endif
