#include <fenv.h>

static unsigned get_mxcsr(void) {
    unsigned m;
    __asm__ volatile("stmxcsr %0" : "=m"(m));
    return m;
}

static void set_mxcsr(unsigned m) {
    __asm__ volatile("ldmxcsr %0" : : "m"(m));
}

int feclearexcept(int excepts) {
    excepts &= FE_ALL_EXCEPT;
    set_mxcsr(get_mxcsr() & ~(unsigned)excepts);
    __asm__ volatile("fnclex");
    return 0;
}

int fetestexcept(int excepts) {
    unsigned short sw;
    __asm__ volatile("fnstsw %0" : "=m"(sw));
    return (int)((get_mxcsr() | sw) & (unsigned)excepts & FE_ALL_EXCEPT);
}

int feraiseexcept(int excepts) {
    set_mxcsr(get_mxcsr() | ((unsigned)excepts & FE_ALL_EXCEPT));
    return 0;
}

int fegetexceptflag(fexcept_t *flag, int excepts) {
    *flag = (fexcept_t)fetestexcept(excepts);
    return 0;
}

int fesetexceptflag(const fexcept_t *flag, int excepts) {
    excepts &= FE_ALL_EXCEPT;
    set_mxcsr((get_mxcsr() & ~(unsigned)excepts) | (*flag & (unsigned)excepts));
    return 0;
}

int fegetround(void) {
    return (int)((get_mxcsr() >> 3) & 0xc00);
}

int fesetround(int round) {
    if (round & ~0xc00)
        return -1;
    unsigned short cw;
    __asm__ volatile("fnstcw %0" : "=m"(cw));
    cw = (unsigned short)((cw & ~0xc00) | round);
    __asm__ volatile("fldcw %0" : : "m"(cw));
    set_mxcsr((get_mxcsr() & ~0x6000u) | ((unsigned)round << 3));
    return 0;
}

int fegetenv(fenv_t *env) {
    __asm__ volatile("fnstenv %0" : "=m"(*env));
    __asm__ volatile("fldenv %0" : : "m"(*env));
    env->__mxcsr = get_mxcsr();
    return 0;
}

int fesetenv(const fenv_t *env) {
    if (env == FE_DFL_ENV) {
        __asm__ volatile("fninit");
        set_mxcsr(0x1f80);
        return 0;
    }
    __asm__ volatile("fldenv %0" : : "m"(*env));
    set_mxcsr(env->__mxcsr);
    return 0;
}

int feholdexcept(fenv_t *env) {
    fegetenv(env);
    feclearexcept(FE_ALL_EXCEPT);
    set_mxcsr(get_mxcsr() | 0x1f80);
    return 0;
}

int feupdateenv(const fenv_t *env) {
    int raised = fetestexcept(FE_ALL_EXCEPT);
    fesetenv(env);
    feraiseexcept(raised);
    return 0;
}
