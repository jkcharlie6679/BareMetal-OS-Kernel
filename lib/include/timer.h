#include "stdint.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define STR(x) #x
#define XSTR(s) STR(s)

typedef void (*callback_typ)(char *);

typedef struct timer_list {
  uint64_t expired_time;
  callback_typ call_back;
  char msg[100];
  struct timer_list *next;
} timer_list;

typedef struct task_list{
  callback_typ task_call_back;
  char *arg;
  uint32_t piority;
  struct task_list *next;
} task_list;

void interrupt_enable(void);
void interrupt_disable(void);
void core_timer_enable(void);
void core_timer_interrupt_enable(void);
void core_timer_interrupt_disable(void);
uint64_t get_timer_tick();
uint64_t get_timer_freq();
uint64_t clock_time_s(void);
void set_core_timer_interrupt(uint64_t expired_time);
void add_timer(callback_typ callback, char *msg, int time);
void pop_timer(void);
void normal_timer();
