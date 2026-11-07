#include <setjmp.h>
#include <signal.h>

void __sigsetjmp_save(sigjmp_buf env, int savemask) {
    env->__saved = savemask;
    if (savemask)
        sigprocmask(SIG_BLOCK, NULL, &env->__mask);
}

void siglongjmp(sigjmp_buf env, int val) {
    if (env->__saved)
        sigprocmask(SIG_SETMASK, &env->__mask, NULL);
    longjmp(env->__jb, val);
}
