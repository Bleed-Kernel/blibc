// link into a shared library, it gives the library its own __dso_handle
.hidden __dso_handle
.globl  __dso_handle
.section .data.rel.ro,"aw"
.balign 8
__dso_handle:
    .quad __dso_handle

.section .note.GNU-stack,"",@progbits
