

SECTIONS
{   
  . = 0x70000;
  _code_relocate_place = .;
  . = 0x80000;
  _start = .;
  .text : { *(.text) }
  
  .bss (NOLOAD) : {
    . = ALIGN(16);
    __bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    __bss_end = .;
  }
  _end = .;
}
__bss_size = (__bss_end - __bss_start) >> 3; 
__code_size = (_end - _start);

/* >>3 == /8 for change the unit to how many 64 bits */
