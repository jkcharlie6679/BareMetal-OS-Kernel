#include "gpio.h"
#include "uart.h"
#include "mailbox.h"
#include "mailbox.h"

volatile unsigned int  __attribute__((aligned(16))) mbox[8];

#define VIDEOCORE_MBOX    (MMIO_BASE+0x0000B880)
#define MAILBOX_READ      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MAILBOX_WRITE     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MAILBOX_STATUS    ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MAILBOX_RESPONSE  0x80000000
#define MAILBOX_FULL      0x80000000
#define MAILBOX_EMPTY     0x40000000

static int mbox_call(unsigned char ch) {
  unsigned int r = (((unsigned int)((unsigned long)&mbox) & (~0xF)) | (ch & 0xf));

  do {
    asm volatile("nop");
  } while (*MAILBOX_STATUS & MAILBOX_FULL); // wait until we can write to the mailbox

  *MAILBOX_WRITE = r; // write the address of our message to the mailbox with channel identifier

  while(1) {
    do {
      asm volatile("nop");
    } while (*MAILBOX_STATUS & MAILBOX_EMPTY);

    if (r == *MAILBOX_READ)
      return mbox[1] == MAILBOX_RESPONSE;
  }
  return 0;
}

int mailbox_property(unsigned int identifier, unsigned int buffer_size, unsigned char ch){
  mbox[0] = (6 + (int)(buffer_size / 4)) * 4; // buffer size in bytes
  mbox[1] = 0x00000000; // REQUEST_CODE
  mbox[2] = identifier; // tag identifier
  mbox[3] = buffer_size; // maximum of request and response value buffer's length.
  mbox[4] = 0x00000000; // TAG_REQUEST_CODE
  mbox[5] = 0; // value buffer
  mbox[6] = 0; // END_TAG
  if (mbox_call(ch))
    return 1;
  return 0; 
}

void mbox_board_ver(uint32_t *board_ver) {
  mailbox_property(MAILBOX_TAG_BOARD_VISION, 4, MBOX_CH_PROP);
  *board_ver = mbox[5];
}

void mbox_mem_info(uint32_t *mem_start_addr, uint32_t *mem_size) {
  mailbox_property(MAILBOX_TAG_MEMORY, 8, MBOX_CH_PROP);
  *mem_start_addr = mbox[5];
  *mem_size = mbox[6];
}
