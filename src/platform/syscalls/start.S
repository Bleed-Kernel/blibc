.global _start
.extern main
.extern _exit
.extern __blibc_set_environ

_start:
    # rdi=argc, rsi=argv
    mov %rdi, %rbx
    mov %rsi, %rcx

    # envp is directly after argv[argc]
    lea 8(%rsi,%rdi,8), %rdx

    mov %rdx, %rdi
    call __blibc_set_environ

    mov %rbx, %rdi
    mov %rcx, %rsi
    call main

    mov %eax, %edi
    call _exit
