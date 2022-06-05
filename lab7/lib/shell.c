#include "shell.h"
#include "uart.h"
#include "string.h"
#include "reboot.h"
#include "printf.h"
#include "cpio.h"
#include "malloc.h"
#include "scheduler.h"
#include "vfs.h"

void shell(){
  char *input = malloc_(sizeof(char) * BUF_MAX_SIZE);
  char **args = malloc_(sizeof(char*) * BUF_ARG_SIZE);
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
        }else if(!strcmp(args[0], "reboot")){
          printf("Bye bye~\n\r");
          reset(300);
        }else if(!strcmp(args[0], "ls")){
          cpio_ls();
        }else if(!strcmp(args[0], "cat")){
          cpio_cat(args[1]);
        }else if(!strcmp(args[0], "r")){
          char *addr = load_program("vfs1.img");
          if(addr != 0){
            task_create((thread_func)addr, USER);
            idle_thread();
          }
        }else if(!strcmp(args[0], "mk")){
          vfs_mkdir(args[1]);
        }else if(!strcmp(args[0], "op")){
          file *open_file = NULL;
          vfs_open(args[1], 0100, &open_file);
        }else if(!strcmp(args[0], "mount")){
          vfs_mount(args[1], "tmpfs");
        }else if(!strcmp(args[0], "dd")){
          vfs_dump_root();
        }
        printf("pi#  ");
        input[0] = 0;
      }
    }
  }
}

