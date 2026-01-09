.global _start
.extern main
.extern _exit

_start:
    mov (%rsp), %rdi        # argc
    lea 8(%rsp), %rsi       # argv
    xor %rdx, %rdx          # null for now

    call main

    mov %eax, %edi
    cli
    hlt
    call _exit