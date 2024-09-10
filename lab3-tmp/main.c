#include "uart.h"
#include "malloc.h"
#include "fdtb.h"
#include "string.h"
#include "reboot.h"
#include "mailbox.h"
#include "printf.h"
#include "cpio.h"
#include "timer.h"

#define BUF_MAX_SIZE 100
#define BUF_ARG_SIZE 100

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

static void shell () {
  char *input = simple_malloc(sizeof(char) * BUF_MAX_SIZE);
  char **args = simple_malloc(sizeof(char*) * BUF_ARG_SIZE);
  int args_num = 0;
  while (1) {
    char read = 0;
    read = async_uart_getc();
    // read = uart_getc();
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
      args_num = spilt_strings(args, input, " ");
      if (args_num != 0) {
        printf("\n\r");
        if (!strcmp(args[0], "help")) {
          printf("help       : print this help menu\n\r");
          printf("hello      : print Hello World!\n\r");
          printf("reboot     : reboot the device\n\r");
          printf("sysinfo    : print the system information\n\r");
          printf("ls         : list the file\n\r");
          printf("cat <file> : print the content of the file\n\r");
          printf("exec <file>: exec the content of the file\n\r");
          printf("clock      : print the core timer time every 2 seconds\n\r");
        } else if(!strcmp(args[0], "hello")) {
          printf("Hello World!\n\r");
        } else if(!strcmp(args[0], "reboot")) {
          printf("Bye bye~\n\r");
          reset(300);
        } else if(!strcmp(args[0], "sysinfo")) {
          print_hardware_info();
        } else if (!strcmp(args[0], "ls")) {
          cpio_ls();
        } else if (!strcmp(args[0], "cat")) {
          cpio_cat(args[1]);
        } else if (!strcmp(args[0], "exec")) {
          if(args_num == 2)
            cpio_exec(args[1]);
        } else if (!strcmp(args[0], "clock")) {
          core_timer_enable();
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
}

void main(char * arg) {

  fdtb_place = arg;

  uart_init();

  __asm__ __volatile__(
    "msr DAIFClr, 0xf" // enable interrupt el1 -> el1 such as core time interrupt
  ); 

  fdt_traverse(initramfs_callback);

  printf("\n\r\n\rWelcome!!!\n\r# ");

  shell();
}
