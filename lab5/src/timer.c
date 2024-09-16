#include "timer.h"
#include "malloc.h"
#include "string.h"
#include "uart.h"
#include "scheduler.h"
#include "stddef.h"
#include "printf.h"
#include "system.h"

static timer_list *timer_queue = NULL;

void interrupt_enable(void){
  asm volatile("msr DAIFClr, 0xf");
}

void interrupt_disable(void){
  asm volatile("msr DAIFSet, 0xf");
}

void core_timer_enable(void){
  /* code from TA lab 5 */
  uint64_t tmp;
  asm volatile("mrs %[input], cntkctl_el1" :[input]"=r"(tmp));
  tmp |= 1;
  asm volatile("msr cntkctl_el1, %[output]" ::[output]"r"(tmp));
  /* enable Counter-timer Physical Timer Control */
  asm volatile("mov x1          , 1\n");
  asm volatile("msr cntp_ctl_el0, x1\n");
}

void core_timer_interrupt_enable(void){
  /* unmask timer interrupt */
  asm volatile("mov x2, 2\n");
  asm volatile("ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n");
  asm volatile("str w2, [x1]\n\t");
}

void core_timer_interrupt_disable(void){
  /* mask timer interrupt */
  asm volatile("mov x2, 0\n");
  asm volatile("ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n");
  asm volatile("str w2, [x1]\n\t");
}

uint64_t get_timer_tick(){
  uint64_t cntpct_el0;
  asm volatile("mrs %[input], cntpct_el0\n":[input]"=r"(cntpct_el0));
  return cntpct_el0;
}

uint64_t get_timer_freq(){
  uint64_t cntfrq_el0;
  asm volatile("mrs %[input], cntfrq_el0\n": [input]"=r"(cntfrq_el0));
  return cntfrq_el0;
}

uint64_t clock_time_s(void){
  return get_timer_tick() / get_timer_freq();
}

void set_core_timer_interrupt(uint64_t expired_time){
  asm volatile("msr cntp_tval_el0, %[output]\n"::[output]"r"(expired_time));
}

void add_timer(callback_typ callback, char *msg, int time) {
  timer_list *timer = (timer_list*)malloc(sizeof(timer_list));
  timer->expired_time = (unsigned long long)time + get_timer_tick();
  timer->call_back = callback;
  for(int i=0; i<=strlen(msg); i++)
    timer->msg[i] = *(msg+i);
  timer->next = NULL;
  if(!timer_queue){               // be the haed
    timer_queue = timer;
    set_core_timer_interrupt(time);
    core_timer_interrupt_enable();
  }else if (timer_queue->expired_time > timer->expired_time) { // add to head
    timer->next = timer_queue;
    timer_queue = timer;
    set_core_timer_interrupt(time);
    core_timer_interrupt_enable();
  }else{                           // insert
    timer_list *pre = timer_queue, *next = timer_queue->next;
    while (next && pre->expired_time < timer->expired_time) {
      pre = next;
      next = next->next;
    }
    pre->next = timer;
    timer->next = next;
  }
}

void pop_timer(void){
  timer_list *timer = timer_queue;
  timer_queue = timer_queue->next;
  timer->call_back(timer->msg);
  if (!timer_queue){
    core_timer_interrupt_disable();
  }else{
    set_core_timer_interrupt(timer_queue->expired_time - get_timer_tick());
    core_timer_interrupt_enable();
  }
  free(timer);
}

void normal_timer(){
  add_timer(normal_timer, "normal_timer", get_timer_freq()>>5); 
  // interrupt_enable();
  schedule();
}
