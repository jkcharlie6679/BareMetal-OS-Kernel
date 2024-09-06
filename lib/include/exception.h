#include "gpio.h"
#include "stdint.h"

// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf p16
#define CORE0_INTERRUPT_SOURCE ((volatile uint64_t *)(0x40000060))
#define IRQS1_PENDING   ((volatile uint64_t *)(MMIO_BASE+0x0000b204))
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)

#define SYSCALL_NUM_PID             0
#define SYSCALL_NUM_UART_READ       1
#define SYSCALL_NUM_UART_WRITE      2
#define SYSCALL_NUM_EXEC            3
#define SYSCALL_NUM_FORK            4
#define SYSCALL_NUM_EXIT            5
#define SYSCALL_NUM_MBOX_CALL       6
#define SYSCALL_NUM_KILL            7
#define SYSCALL_NUM_REGISTER        8
#define SYSCALL_NUM_SIGNAL_KILL     9
#define SYSCALL_NUM_OPEN            11
#define SYSCALL_NUM_CLOSE           12
#define SYSCALL_NUM_WRITE           13
#define SYSCALL_NUM_READ            14
#define SYSCALL_NUM_MKDIR           15
#define SYSCALL_NUM_MOUNT           16
#define SYSCALL_NUM_CHDIR           17


void invalid_exception_router(uint64_t x0);
void irq_router(uint64_t x0);
void sync_router(uint64_t x0, uint64_t x1);
void child_return_from_fork();
// void lower_irq_router(uint64_t x0);

typedef struct trap_frame{
  uint64_t x0;        uint64_t x1;
  uint64_t x2;        uint64_t x3;
  uint64_t x4;        uint64_t x5;
  uint64_t x6;        uint64_t x7;
  uint64_t x8;        uint64_t x9;
  uint64_t x10;       uint64_t x11;
  uint64_t x12;       uint64_t x13;
  uint64_t x14;       uint64_t x15;
  uint64_t x16;       uint64_t x17;
  uint64_t x18;       uint64_t x19;
  uint64_t x20;       uint64_t x21;
  uint64_t x22;       uint64_t x23;
  uint64_t x24;       uint64_t x25;
  uint64_t x26;       uint64_t x27;
  uint64_t x28;       uint64_t x29;
  uint64_t x30;       uint64_t padd1;
  uint64_t sp_el0;    uint64_t elr_el1;   
  uint64_t spsr_el1;  uint64_t padd2;
} trap_frame;

typedef void (*handler_func)();

// extern handler_func _handler;
// extern uint64_t _pid;
