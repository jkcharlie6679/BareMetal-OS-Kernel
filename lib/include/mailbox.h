#include "gpio.h"

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

#define MAILBOX_READ       ((volatile unsigned int*)(MMIO_BASE+0x0000B880)) 
#define MAILBOX_STATUS     ((volatile unsigned int*)(MMIO_BASE+0x0000B898)) 
#define MAILBOX_WRITE      ((volatile unsigned int*)(MMIO_BASE+0x0000B8A0)) 
#define MAILBOX_RESPONSE   0x80000000
#define MAILBOX_FULL       0x80000000
#define MAILBOX_EMPTY      0x40000000

// volatile unsigned int  __attribute__((aligned(16))) mbox[8];
int mbox_call(unsigned char ch, unsigned int *mbox);
