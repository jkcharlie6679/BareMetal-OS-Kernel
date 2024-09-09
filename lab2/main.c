#include "uart.h"
#include "malloc.h"
#include "fdtb.h"
#include "string.h"
#include "reboot.h"
#include "mailbox.h"
#include "printf.h"
#include "cpio.h"

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
  char input[20] = "";
  while (1) {
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
          printf("help      : print this help menu\n\r");
            printf("hello     : print Hello World!\n\r");
            printf("reboot    : reboot the device\n\r");
            printf("sysinfo   : print the system information\n\r");
            printf("ls        : list the file\n\r");
            printf("cat <file>: print the content of the file\n\r");
        } else if(!strcmp(input, "hello")) {
          printf("Hello World!\n\r");
        } else if(!strcmp(input, "reboot")) {
          printf("Bye bye~\n\r");
          reset(300);
        } else if(!strcmp(input, "sysinfo")) {
          print_hardware_info();
        } else if (!strcmp(input, "ls")) {
          cpio_ls();
        } else if ((input[0] == 'c') && (input[1] == 'a') && (input[2] == 't') && (input[3] == 0x20)) {
          cpio_cat(input + 4);
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

// void shell(){
//   char input[20] = "";
//   while(1){
//     char read = 0;
//     read = uart_getc();
//     if(read != '\n' && read != 0x7f){
//       append_str(input, read);
//       printf("%c", read);
//       read = 0;
//     }else if(read == 0x7f){
//       if(strlen(input) > 0){
//         pop_str(input);
//         printf("\b \b");
//       }
//     }else{
//       if(strlen(input) != 0){
//         printf("\n\r");
//         if(!strcmp(input, "help")){
//           printf("help      : print this help menu\n\r");
//           printf("hello     : print Hello World!\n\r");
//           printf("reboot    : reboot the device\n\r");
//           printf("sysinfo   : print the system information\n\r");
//           printf("ls        : list the file\n\r");
//           printf("cat <file>: print the content of the file\n\r");
//         }else if(!strcmp(input, "hello")){
//           printf("Hello World!\n\r");
//         }else if(!strcmp(input, "reboot")){
//           printf("Bye bye~\n\r");
//           reset(300);
//         }else if(!strcmp(input, "sysinfo")){
//           print_hardware_info();
//         }else if(!strcmp(input, "ls")){
//           cpio_ls();
//         }else if((input[0] == 'c') && (input[1] == 'a') && (input[2] == 't') && (input[3] == 0x20)){
//           cpio_cat(input + 4);
//         }else{
//           printf("Please use \"help\" to get information.\n\r");
//         }
//         printf("# ");
//         input[0] = 0;
//       }else{
//         printf("\n\r# ");
//       }  
//     }
//   }
// }


void main (char * arg) {

  uart_init();

  fdtb_place = arg;
  fdt_traverse(initramfs_callback);

  // Test the simple_malloc
  char* test1 = simple_malloc(0x18);
  test1 = "Test the malloc 1.";
  printf("%s\n\r", test1);
  char* test2 = simple_malloc(0x32);
  test2 = "Test the malloc 2.";
  printf("%s\n\r", test2);
  char* test3 = simple_malloc(0x36);
  test3 = "Test the malloc 3.";
  printf("%s\n\r", test3);

  printf("\n\r\n\rWelcome!!!\n\r# ");

  shell();
}
