#include "uart.h"
#include "shell.h"
#include "printf.h"

int main() {
  uart_init();
  char input[20] = "";
  printf("\n\r\n\rWelcome!!! \n\r# ");

  while (1) {
    shell(input);
  }

  return 0;
}
