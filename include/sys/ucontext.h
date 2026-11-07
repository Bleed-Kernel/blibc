#pragma once

#include <stdint.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NGREG 23
typedef long long greg_t, gregset_t[NGREG];

enum {
    REG_R8 = 0, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15,
    REG_RDI, REG_RSI, REG_RBP, REG_RBX, REG_RDX, REG_RAX, REG_RCX, REG_RSP,
    REG_RIP, REG_EFL, REG_CSGSFS, REG_ERR, REG_TRAPNO, REG_OLDMASK, REG_CR2
};
#define REG_R8 REG_R8
#define REG_R9 REG_R9
#define REG_R10 REG_R10
#define REG_R11 REG_R11
#define REG_R12 REG_R12
#define REG_R13 REG_R13
#define REG_R14 REG_R14
#define REG_R15 REG_R15
#define REG_RDI REG_RDI
#define REG_RSI REG_RSI
#define REG_RBP REG_RBP
#define REG_RBX REG_RBX
#define REG_RDX REG_RDX
#define REG_RAX REG_RAX
#define REG_RCX REG_RCX
#define REG_RSP REG_RSP
#define REG_RIP REG_RIP
#define REG_EFL REG_EFL
#define REG_CSGSFS REG_CSGSFS
#define REG_ERR REG_ERR
#define REG_TRAPNO REG_TRAPNO
#define REG_OLDMASK REG_OLDMASK
#define REG_CR2 REG_CR2

struct _libc_fpxreg {
    unsigned short significand[4];
    unsigned short exponent;
    unsigned short padding[3];
};

struct _libc_xmmreg {
    uint32_t element[4];
};

struct _libc_fpstate {
    uint16_t cwd, swd, ftw, fop;
    uint64_t rip, rdp;
    uint32_t mxcsr, mxcr_mask;
    struct _libc_fpxreg _st[8];
    struct _libc_xmmreg _xmm[16];
    uint32_t padding[24];
};
typedef struct _libc_fpstate *fpregset_t;

typedef struct {
    gregset_t gregs;
    fpregset_t fpregs;
    unsigned long long __reserved1[8];
} mcontext_t;

struct sigcontext {
    unsigned long r8, r9, r10, r11, r12, r13, r14, r15;
    unsigned long rdi, rsi, rbp, rbx, rdx, rax, rcx, rsp, rip, eflags;
    unsigned short cs, gs, fs, __pad0;
    unsigned long err, trapno, oldmask, cr2;
    struct _libc_fpstate *fpstate;
    unsigned long __reserved1[8];
};

typedef struct __ucontext {
    unsigned long uc_flags;
    struct __ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    sigset_t uc_sigmask;
    unsigned long __fpregs_mem[64];
} ucontext_t;

#ifdef __cplusplus
}
#endif
