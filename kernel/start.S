.section .text.start
.globl kstart
.type kstart, @function
.extern kmain

kstart:
    cli
    movl $stack_top, %esp

    call enable_sse         /* <--- ADD THIS CALL HERE */
    
    push %ebx
    call kmain

1:
    hlt
    jmp 1b

/* --- ADD THIS FUNCTION --- */
enable_sse:
    /* 1. Update CR0 */
    movl %cr0, %eax
    andl $0xFFFB, %eax      /* Clear Coprocessor Emulation (EM) bit (Bit 2) */
    orl  $0x2, %eax         /* Set Monitor Coprocessor (MP) bit (Bit 1) */
    movl %eax, %cr0

    /* 2. Update CR4 */
    movl %cr4, %eax
    orl  $0x600, %eax       /* Set OSFXSR (Bit 9) and OSXMMEXCPT (Bit 10) */
    movl %eax, %cr4
    ret

.section .bss
.align 16
stack:
    .skip 16384
stack_top:
