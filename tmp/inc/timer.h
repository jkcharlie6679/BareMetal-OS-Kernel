#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define STR(x) #x
#define XSTR(s) STR(s)
#define CORE0_TIMER_IRQ_CTRL 0x40000040

typedef void (*callback_typ)(char *);

typedef struct timer_list {
  unsigned long long expired_time;
  callback_typ call_back;
  char msg[100];
  struct timer_list *next;
} timer_list;

typedef struct task_list{
  callback_typ task_call_back;
  char *arg;
  int piority;
  struct task_list *next;
} task_list;

void interrupt_enable(void);
void interrupt_disable(void);
void core_timer_enable(void);
void core_timer_interrupt_enable(void);
void core_timer_interrupt_disable(void);
void clock_alert(char *str);
void timeout_print(char *str);

void add_timer(callback_typ callback, char *msg, int time);
void pop_timer(void);

#ifdef __cplusplus
}
#endif

#endif
