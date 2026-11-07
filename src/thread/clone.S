.globl __thread_clone
.type  __thread_clone,@function
__thread_clone:
    mov  8(%rsp), %rax
    sub  $16, %rsi
    mov  %r9, (%rsi)
    mov  %rax, 8(%rsi)

    mov  %rcx, %r10
    mov  $56, %eax
    syscall
    test %rax, %rax
    jnz  1f

    xor  %ebp, %ebp
    pop  %rax
    pop  %rdi
    and  $-16, %rsp
    call *%rax

    mov  %eax, %edi
    mov  $60, %eax
    syscall
    hlt
1:
    ret
.size __thread_clone, .-__thread_clone

.globl __thread_unmap_and_exit
.type  __thread_unmap_and_exit,@function
__thread_unmap_and_exit:
    mov  $11, %eax
    syscall
    xor  %edi, %edi
    mov  $60, %eax
    syscall
    hlt
.size __thread_unmap_and_exit, .-__thread_unmap_and_exit

.section .note.GNU-stack,"",@progbits
