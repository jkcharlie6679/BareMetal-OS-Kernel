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

char* CMD[][2] = {
  {"help                       ", "print this help menu"},
  {"hello                      ", "print Hello World!"},
  {"reboot                     ", "reboot the device"},
  {"sysinfo                    ", "print the system information"},
  {"ls                         ", "list the file"},
  {"cat <file>                 ", "print the content of the file"},
  {"exec <file>                ", "exec the content of the file"},
  {"clock                      ", "print the core timer time every 2 seconds"},
  {"time <MESSAGE> <TIME>", "Print the message when timeout"},
  {"alloc <size>               ", "memory allocate"},
  {"free <addr>                ", "free the memory"},
};

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
  char *input = malloc(sizeof(char) * BUF_MAX_SIZE);
  char **args = malloc(sizeof(char*) * BUF_ARG_SIZE);
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
          for (int i = 0; sizeof(CMD) && i < sizeof(CMD) / sizeof(CMD[0]); ++i)
            printf("%s: %s\n", CMD[i][0], CMD[i][1]);
        } 
        else if(!strcmp(args[0], "hello")) {
          printf("Hello World!\n\r");
        } 
        else if(!strcmp(args[0], "reboot")) {
          printf("Bye bye~\n\r");
          reset(300);
        } 
        else if(!strcmp(args[0], "sysinfo")) {
          print_hardware_info();
        } 
        else if (!strcmp(args[0], "ls")) {
          cpio_ls();
        } 
        else if (!strcmp(args[0], "cat")) {
          cpio_cat(args[1]);
        } 
        else if (!strcmp(args[0], "exec")) {
          if(args_num == 2)
            cpio_exec(args[1]);
        } 
        else if (!strcmp(args[0], "clock")) {
          add_timer(clock_alert, "clock_alert", 2);
        } 
        else if (!strcmp(args[0], "alloc")) {
          if (args_num == 2)
            printf("%x\n", malloc(atoi(args[1])));
        } 
        else if (!strcmp(args[0], "free")) {
          if (args_num == 2)
            free((void *)(uint64_t)atoi(args[1]));
        } 
        else if (!strcmp(input, "time")) {
          if (args_num == 3)
            add_timer(timeout_print, args[1], atoi(args[2]));
        } 
        else {
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
  interrupt_enable(); // el0 -> el0
  core_timer_enable();

  fdt_traverse(initramfs_callback);

  page_init();

  printf("\n\nWelcome!!!\n# ");

  shell();
}
