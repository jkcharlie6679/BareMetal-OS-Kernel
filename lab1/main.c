#include <stdint.h>
#include "uart.h"
#include "printf.h"
#include "string.h"
#include "uart.h"
#include "mailbox.h"
#include "reboot.h"


static void print_hardware_info () {
  uint32_t board_ver = 0;
  uint32_t mem_start_addr = 0;
  uint32_t mem_size = 0;
  mbox_board_ver(&board_ver);
  mbox_mem_info(&mem_start_addr, &mem_size);
  printf("ARM memory size: 0x%08x\n\r", board_ver);
  printf("ARM memory base address: 0x%08x\n\r", mem_start_addr);
  printf("ARM memory size: 0x%08x\n\r", mem_size);
}


static void shell (char *input) {
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
        print_hardware_info();
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

int main() {
  uart_init();
  char input[20] = "";
  printf("\n\r\n\rWelcome!!! \n\r# ");

  while (1) {
    shell(input);
  }

  return 0;
}
