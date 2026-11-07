#ifdef BLIBC_SHARED
.globl _dlstart
.hidden __dl_start
.type  _dlstart,@function
_dlstart:
    mov  %rsp, %r12
    mov  %rsp, %rdi
    and  $-16, %rsp
    call __dl_start

    mov  %r12, %rsp
    mov  (%rsp), %rdi
    lea  8(%rsp), %rsi
    lea  16(%rsp,%rdi,8), %rdx
    xor  %ebp, %ebp
    jmp  *%rax
.size _dlstart, .-_dlstart
#endif

.section .note.GNU-stack,"",@progbits
