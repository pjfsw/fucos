ENTRY(kstart)

SECTIONS
{
  . = 0x00100000;

  .text : {
    *(.text.start)
    *(.text*)
  }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(COMMON) *(.bss*) }
}
