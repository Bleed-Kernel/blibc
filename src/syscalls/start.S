.global _start
.extern main
.extern _exit

_start:
    xor %rdx, %rdx          # null for now
    call main

    mov %eax, %edi
    cli
    hlt
    call _exit
