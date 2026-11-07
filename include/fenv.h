#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define FE_INVALID    0x01
#define FE_DIVBYZERO  0x04
#define FE_OVERFLOW   0x08
#define FE_UNDERFLOW  0x10
#define FE_INEXACT    0x20
#define FE_ALL_EXCEPT 0x3f

#define FE_TONEAREST  0x000
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xc00

typedef unsigned short fexcept_t;

typedef struct {
    unsigned short __control_word;
    unsigned short __unused1;
    unsigned short __status_word;
    unsigned short __unused2;
    unsigned short __tags;
    unsigned short __unused3;
    unsigned int   __eip;
    unsigned short __cs_selector;
    unsigned int   __opcode : 11;
    unsigned int   __unused4 : 5;
    unsigned int   __data_offset;
    unsigned short __data_selector;
    unsigned short __unused5;
    unsigned int   __mxcsr;
} fenv_t;

#define FE_DFL_ENV ((const fenv_t *)-1)

int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flag, int excepts);
int feraiseexcept(int excepts);
int fesetexceptflag(const fexcept_t *flag, int excepts);
int fetestexcept(int excepts);
int fegetround(void);
int fesetround(int round);
int fegetenv(fenv_t *env);
int feholdexcept(fenv_t *env);
int fesetenv(const fenv_t *env);
int feupdateenv(const fenv_t *env);

#ifdef __cplusplus
}
#endif
