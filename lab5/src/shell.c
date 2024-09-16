#include "shell.h"
#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "printf.h"
#include "cpio.h"
#include "malloc.h"
#include "scheduler.h"
#include "timer.h"


void test(){
  printf("test\n\r");
}

void shell(){
  char *input = malloc(sizeof(char) * BUF_MAX_SIZE);
  char **args = malloc(sizeof(char*) * BUF_ARG_SIZE);
  *input = 0;
  int args_num = 0;
  char read = 0;
  printf("pi#  ");
  while(1){
    read = uart_getc();
    if(read != '\n' && read != 0x7f){
      append_str(input, read);
      printf("%c", read);
    }else if(read == 0x7f){
      if(strlen(input) > 0){
        pop_str(input);
        printf("\b \b");
      }
    }else{
      args_num = spilt_strings(args, input, " ");
      if(args_num != 0){
        printf("\n\r");
        if(!strcmp(args[0], "help")){
          printf("reboot    : reboot the device\n\r");
          printf("exec <file>: exec the content of the file\n\r");
        }else if(!strcmp(args[0], "reboot")){
          printf("Bye bye~\n\r");
          reset(300);
        }else if(!strcmp(args[0], "ls")){
          cpio_ls();
        }else if(!strcmp(args[0], "cat")){
          cpio_cat(args[1]);
        }else if(!strcmp(args[0], "run")){
          char *addr = load_program("syscall.img");
          if(addr != 0){
            task_create((thread_func)addr, USER);
            idle_thread();
          }
        }else if(!strcmp(args[0], "t")){
          for(int i = 0; i < 3; ++i) { 
            task_create(foo, USER);
          }
          idle_thread();
        }else if(!strcmp(args[0], "f")){
          task_create(fork_test, USER);
          idle_thread();
        }
        printf("pi#  ");
        input[0] = 0;
      }
    }
  }
}

