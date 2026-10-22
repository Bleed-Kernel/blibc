.global _start
.extern main
.extern exit
.extern __blibc_set_environ

_start:
    # rdi=argc, rsi=argv
    mov %rdi, %rbx
    mov %rsi, %r12

    xor %edi, %edi
    call __blibc_set_environ

    mov %rbx, %rdi
    mov %r12, %rsi
    call main

    mov %eax, %edi
    call exit
