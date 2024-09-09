#include "printf.h"
#include "string.h"
#include "uart.h"
#include "mailbox.h"
#include "reboot.h"

void shell (char *input) {
  char read = 0;
  read = uart_getc();
  if (read != '\n' && read != 0x7f) { // 0x7f delete
    append_str(input, read);
    printf("%c", read);
  } else if(read == 0x7f) {
    read = 0;
    if (strlen(input) > 0) {
      pop_str(input);
      printf("\b \b");
    }
  } else {
    if (strlen(input) != 0) {
      printf("\n\r");
      if (!strcmp(input, "help")) {
        printf("help    : print this help menu\n\r");
        printf("hello   : print Hello World!\n\r");
        printf("reboot  : reboot the device\n\r");
        printf("sysinfo : print the system information\n\r");
      } else if(!strcmp(input, "hello")) {
        printf("Hello World!\n\r");
      } else if(!strcmp(input, "reboot")) {
        printf("Bye bye~\n\r");
        reset(300);
      } else if(!strcmp(input, "sysinfo")) {

        if (mailbox_property(MAILBOX_TAG_BOARD_VISION, 4, MBOX_CH_PROP))
          printf("Board version: 0x%08x\n\r", mbox[5]);
        else
          printf("Board version: querry error.\n\r");

        if (mailbox_property(MAILBOX_TAG_MEMORY, 8, MBOX_CH_PROP)) {
          printf("ARM memory base address: 0x%08x\n\r", mbox[5]);
          printf("ARM memory size: 0x%08x\n\r", mbox[6]);
        } else {
          printf("ARM memory base address: querry error.\n\r");
          printf("ARM memory size: querry error.\n\r");
        }
      } else {
        printf("Please use \"help\" to get information.\n\r");
      }
      printf("# ");
      input[0] = 0;
    } else {
      printf("\n\r# ");
    }
  }
}
