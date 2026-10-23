.global _start
.extern main
.extern exit
.extern __blibc_set_environ
.extern __blibc_init_tls

_start:
    # rdi=argc, rsi=argv, rdx=envp
    mov %rdi, %rbx
    mov %rsi, %r12
    mov %rdx, %r13

    call __blibc_init_tls

    mov %r13, %rdi
    call __blibc_set_environ

    mov %rbx, %rdi
    mov %r12, %rsi
    mov %r13, %rdx
    call main

    mov %eax, %edi
    call exit
