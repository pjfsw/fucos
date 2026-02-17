ENTRY(kmain)

SECTIONS
{
  . = 0x00020000;

  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(COMMON) *(.bss*) }
}
