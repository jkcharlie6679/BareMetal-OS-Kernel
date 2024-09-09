#include <stdint.h>
#include "uart.h"
#include "printf.h"

extern char *_code_relocate_place;
extern uint64_t __code_size;
extern uint64_t _start;

void code_relocate(char * addr, char * arg){
  uint64_t size = (uint64_t)&__code_size;
  char* start = (char *)&_start;

  for (uint64_t i = 0; i < size; i++)
    addr[i] = start[i];

  void (*relocation)(char *)  = (void (*) (char *))addr; 
  relocation(arg);
}

int relocate = 1;

void main (char* arg) {
  uart_init();
  char* reloc_place = (char*)&_code_relocate_place;

  if (relocate) {
    relocate = 0;
    code_relocate(reloc_place, arg);
  }

  uint64_t kernel_size = 0;
  char* kernel_start = (char*) 0x80000;
  
  printf("Welcome UART bootloader!!\n\r");

  for (int i = 0; i < 4; i++) {
    kernel_size += uart_getc_pure() << (i * 8);
  }
  for (int i = 0; i < kernel_size; i++) {
    kernel_start[i] = uart_getc_pure();
  }

  void (*kernel) (char *) = (void (*) (char *))kernel_start;
  kernel(arg);
}
