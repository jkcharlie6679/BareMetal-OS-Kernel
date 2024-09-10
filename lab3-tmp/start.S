
.section ".text"

.global _start

_start:
  b       2f
1:  
  wfe
  b       1b
2:  
  bl      from_el2_to_el1
  // set top of stack just before our code (stack grows to a lower address per AAPCS64)
  ldr     x1, =_start
  mov     sp, x1

  // clear bss
  ldr     x1, =__bss_start
  ldr     w2, =__bss_size
3:
  cbz     w2, 4f
  str     xzr, [x1], #8 // put xzr to x1 then x1 = x1 + 8  xzr zero register
  sub     w2, w2, #1
  cbnz    w2, 3b
    
4:
  bl      main
  b       1b

from_el2_to_el1:
  mov x1, (0b11 << 20) // this fixes printf (printf use stdargs, which access register q0~q7, and those are SIMD registers)
  msr cpacr_el1, x1
  mov x1, (1 << 31) // EL1 uses aarch64
  msr hcr_el2, x1
  mov x1, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
  msr spsr_el2, x1
  msr elr_el2, lr
  eret // return to EL1