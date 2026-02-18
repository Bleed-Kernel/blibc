#include "math.h"

double sqrt(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fsqrt  \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double sin(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fsin   \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double cos(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fcos   \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}

double tan(double x) {
    double s, c;
    __asm__ __volatile__ (
        "fldl %2 \n\t"
        "fsin    \n\t"
        "fstpl %0 \n\t"
        "fldl %2 \n\t"
        "fcos    \n\t"
        "fstpl %1"
        : "=m"(s), "=m"(c)
        : "m"(x)
    );
    return s / c;
}

double fabs(double x) {
    double result;
    __asm__ __volatile__ (
        "fldl %1 \n\t"
        "fabs   \n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(x)
    );
    return result;
}