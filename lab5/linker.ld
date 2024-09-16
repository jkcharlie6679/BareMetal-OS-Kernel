

SECTIONS
{
  . = 0x80000;
  _kernel_start = .;
  .text : { *(.text) }
  
  .bss (NOLOAD) : {
    . = ALIGN(16);
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    __bss_end = .;
  }
  _kernel_end = .;
  . = ALIGN(16);
  _heap_start = .;
  _end = .;
}
__bss_size = (__bss_end - __bss_start)>>3; 

/* >>3 == /8 for change the unit to how many 64 bits */
