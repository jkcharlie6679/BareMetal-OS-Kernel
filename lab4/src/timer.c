#include "timer.h"
#include "printf.h"
#include "malloc.h"
#include "string.h"

static timer_list *timer_queue = NULL;

void interrupt_enable(void) {
  asm volatile("msr DAIFClr, 0xf"); // el0 -> el0
}

void interrupt_disable(void) {
  asm volatile("msr DAIFSet, 0xf"); // el0 -> el0
}

void core_timer_enable(void) {
  __asm__ __volatile__(
    "mov x1, 1\n\t"
    "msr cntp_ctl_el0, x1\n\t" // enable Counter-timer Physical Timer Control
  );
}

void core_timer_interrupt_enable(void) {
  __asm__ __volatile__(
    "mov x2, 2\n\t"
    "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
    "str w2, [x1]\n\t" // unmask timer interrupt
  );
}

void core_timer_interrupt_disable(void) {
  __asm__ __volatile__(
    "mov x2, 0\n\t"
    "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
    "str w2, [x1]\n\t" // unmask timer interrupt
  );
}

static void set_core_timer_interrupt(uint64_t expired_time) {
  __asm__ __volatile__(
    "mrs x1, cntfrq_el0\n\t" // cntfrq_el0 -> relative time
    "mul x1, x1, %[output0]\n\t"
    "msr cntp_tval_el0, x1\n\t" // set expired time
    :: [output0] "r" (expired_time)
  );
}

static uint64_t clock_time(void) {
  uint64_t cntpct_el0;
  __asm__ __volatile__(
    "mrs %0, cntpct_el0\n\t"
    : "=r"(cntpct_el0)
  ); // tick now

  uint64_t cntfrq_el0;
  __asm__ __volatile__(
    "mrs %0, cntfrq_el0\n\t"
    : "=r"(cntfrq_el0)
  ); // tick frequency
  return cntpct_el0 / cntfrq_el0;
}

void clock_alert(char *str) {
  printf("seconds after booting : %d\n\r", clock_time());
  printf("Message: %s.\n\r", str);
  add_timer(clock_alert, "clock_alert", 2);
}

void timeout_print(char *str) {
  printf("seconds after booting : %d\n\r", clock_time());
  printf("Message: %s.\n\r", str);
}


void add_timer(callback_typ callback, char *msg, int time) {
  timer_list *timer = (timer_list*)malloc(sizeof(timer_list));
  timer->expired_time = (uint64_t)time + clock_time();
  timer->call_back = callback;
  timer->next = NULL;
  for (int i = 0; i <= strlen(msg); i++)
    timer->msg[i] = *(msg + i);
  if (!timer_queue) { // be the haed
    timer_queue = timer;
    set_core_timer_interrupt(time);
  } else if (timer_queue->expired_time > timer->expired_time) { // add to head
    timer->next = timer_queue;
    timer_queue = timer;
    set_core_timer_interrupt(time);
  } else{ // insert
    timer_list *pre = timer_queue, *next = timer_queue->next;
    while (next && pre->expired_time < timer->expired_time) {
      pre = next;
      next = next->next;
    }
    pre->next = timer;
    timer->next = next;
  }
  core_timer_interrupt_enable();
}

void pop_timer(void) {
  timer_list *timer = timer_queue;
  timer_queue = timer_queue->next;
  timer->call_back(timer->msg);
  if (!timer_queue)
    core_timer_interrupt_disable();
  else{
    set_core_timer_interrupt(timer_queue->expired_time - clock_time());
    core_timer_interrupt_enable();
  }
}
