#include "exception.h"
#include "printf.h"
#include "timer.h"

void invalid_exception_router(uint64_t x0) {
  uint64_t elr_el1, esr_el1, spsr_el1;
  __asm__ __volatile__("mrs %[output0], elr_el1\n\t"
                       "mrs %[output1], ESR_EL1\n\t"
                       "mrs %[output2], SPSR_EL1\n\t"
                       : [output0] "=r" (elr_el1), [output1] "=r" (esr_el1), [output2] "=r" (spsr_el1)
                       :
                       : );
  printf("elr_el1 : 0x%x\r\n", elr_el1);
  printf("esr_el1 : 0x%x\r\n", esr_el1);
  printf("spsr_el1 : 0x%x\r\n", spsr_el1);
  printf("exception number: 0x%x\r\n",x0);
}

void irq_router(uint64_t x0) {
  if (*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ) {
    clock_alert();
  } 
}
