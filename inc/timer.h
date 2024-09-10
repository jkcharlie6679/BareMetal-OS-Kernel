#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define STR(x) #x
#define XSTR(s) STR(s)
#define CORE0_TIMER_IRQ_CTRL 0x40000040

void core_timer_enable(void);
void clock_alert(void);

#endif