#include "mailbox.h"
#include "stdint.h"

int mbox_call(unsigned char ch, unsigned int *mbox){
  unsigned int r = (((uint32_t)((uint64_t)mbox)&(~0xF)) | (ch&0xf));
  do{
    asm volatile("nop");
  }while(*MAILBOX_STATUS & MAILBOX_FULL);     // wait until we can write to the mailbox
  *MAILBOX_WRITE = r;                         // write the address of our message to the mailbox with channel identifier
  while(1) {
    do{
      asm volatile("nop");
    }while(*MAILBOX_STATUS & MAILBOX_EMPTY);
    if(r == *MAILBOX_READ)
      return mbox[1]==MAILBOX_RESPONSE;    
  }
  return 0;
}
