#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdint.h>

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)

void invalid_exception_router(uint64_t x0);
void irq_router(uint64_t x0);

#endif