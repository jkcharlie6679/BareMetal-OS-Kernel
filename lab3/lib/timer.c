#include "timer.h"
#include "printf.h"

void core_timer_enable(void){
  __asm__ __volatile__(
    "mov x1, 1\n\t"
    "msr cntp_ctl_el0, x1\n\t" // enable
    "mrs x0, cntfrq_el0\n\t"
    "mov x1, 2\n\t"
    "mul x0, x0, x1\n\t"
    "msr cntp_tval_el0, x0\n\t" // set expired time
    "mov x2, 2\n\t"
    "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
    "str w2, [x1]\n\t" // unmask timer interrupt
  );
}

void clock_alert(void){
  unsigned long long cntpct_el0;
  __asm__ __volatile__(
    "mrs %0, cntpct_el0\n\t"
    : "=r"(cntpct_el0)
  ); //tick now

  unsigned long long cntfrq_el0;
  __asm__ __volatile__(
    "mrs %0, cntfrq_el0\n\t"
    : "=r"(cntfrq_el0)
  ); //tick frequency

  printf("seconds after booting : %d\r\n", cntpct_el0 / cntfrq_el0);
  set_core_timer_interrupt(2);
}


void set_core_timer_interrupt(unsigned long long expired_time){
  __asm__ __volatile__(
    "mrs x1, cntfrq_el0\n\t" //cntfrq_el0 -> relative time
    "mul x1, x1, %0\n\t"
    "msr cntp_tval_el0, x1\n\t" // set expired time
    : "=r"(expired_time)
  );
}
