ENTRY(kstart)

SECTIONS
{
  . = 0x00100000;
  _kernel_start = . ;

  .text : {
    *(.text.start)
    *(.text*)
  }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(COMMON) *(.bss*) }

  . = ALIGN(4096);
  _kernel_end = . ;
}
