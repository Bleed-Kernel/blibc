#include <pthread.h>
#include <errno.h>
#include <string.h>

int pthread_attr_init(pthread_attr_t *attr) {
    memset(attr, 0, sizeof(*attr));
    attr->stack_size = 512 * 1024;
    attr->guard_size = 4096;
    attr->inherit_sched = PTHREAD_INHERIT_SCHED;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int state) {
    if (state != PTHREAD_CREATE_JOINABLE && state != PTHREAD_CREATE_DETACHED)
        return EINVAL;
    attr->detached = state;
    return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *state) {
    *state = attr->detached;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size) {
    if (size < PTHREAD_STACK_MIN)
        return EINVAL;
    attr->stack_size = size;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *size) {
    *size = attr->stack_size;
    return 0;
}

// no guard pages yet, its kept so it reads back
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t size) {
    attr->guard_size = size;
    return 0;
}

int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *size) {
    *size = attr->guard_size;
    return 0;
}

int pthread_attr_setstack(pthread_attr_t *attr, void *addr, size_t size) {
    if (size < PTHREAD_STACK_MIN)
        return EINVAL;
    attr->stack_addr = addr;
    attr->stack_size = size;
    return 0;
}

int pthread_attr_getstack(const pthread_attr_t *attr, void **addr, size_t *size) {
    *addr = attr->stack_addr;
    *size = attr->stack_size;
    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *addr) {
    attr->stack_addr = addr;
    return 0;
}

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **addr) {
    *addr = attr->stack_addr;
    return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) {
    if (policy != SCHED_OTHER)
        return ENOTSUP;
    attr->sched_policy = policy;
    return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy) {
    *policy = attr->sched_policy;
    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param) {
    attr->sched_priority = param->sched_priority;
    return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param) {
    param->sched_priority = attr->sched_priority;
    return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit) {
    if (inherit != PTHREAD_INHERIT_SCHED && inherit != PTHREAD_EXPLICIT_SCHED)
        return EINVAL;
    attr->inherit_sched = inherit;
    return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit) {
    *inherit = attr->inherit_sched;
    return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int scope) {
    if (scope != PTHREAD_SCOPE_SYSTEM)
        return ENOTSUP;
    attr->scope = scope;
    return 0;
}

int pthread_attr_getscope(const pthread_attr_t *attr, int *scope) {
    *scope = attr->scope;
    return 0;
}
