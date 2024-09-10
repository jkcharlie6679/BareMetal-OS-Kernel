#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdint.h>
#include "gpio.h"

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)
#define IRQS1_PENDING   ((volatile unsigned int*)(MMIO_BASE+0x0000b204))

void invalid_exception_router(uint64_t x0);
void irq_router(uint64_t x0);

#endif