.globl _start
.type  _start,@function
_start:
    xor  %ebp, %ebp
    call main
    mov  %eax, %edi
    call exit@PLT
    hlt
.size _start, .-_start

// what __cxa_atexit is told the program acc is
.hidden __dso_handle
.globl  __dso_handle
.section .data.rel.ro,"aw"
.balign 8
__dso_handle:
    .quad __dso_handle

.section .note.GNU-stack,"",@progbits
