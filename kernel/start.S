.section .text.start
.globl kstart
.type kstart, @function
.extern kmain

kstart:
    cli
    movl $stack_top, %esp
    push %ebx
    call kmain

1:
    hlt
    jmp 1b

.section .bss
.align 16
stack:
    .skip 16384
stack_top:
