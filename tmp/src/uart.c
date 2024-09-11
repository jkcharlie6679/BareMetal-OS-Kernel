#include "uart.h"
#include "uart.h"
#include "gpio.h"

void uart_init() {
  register unsigned int r;

  r = *GPFSEL1;
  r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15
  r |= (2<<12) | (2<<15);        // alt5  010010
  *GPFSEL1 = r;

  *GPPUD = 0;                    // disable pull up/down

  r = 150; 
  while(r--)
    asm volatile("nop");

  *GPPUDCLK0 = (1 << 14) | (1 << 15);

  r = 150; 
  while(r--) 
    asm volatile("nop"); 

  *GPPUDCLK0 = 0;        // flush GPIO setup

  *AUX_ENABLE |= 1;      // enable UART1, AUX mini uart
  *AUX_MU_CNTL = 0;      // disable Tx, Rx when configuration
  *AUX_MU_IER = 0;       // enable interrupt, no use read interrupt so use 2, if want to enable read interrupt plz use 3
  *AUX_MU_LCR = 3;       // set 8 bits
  *AUX_MU_MCR = 0;
  *IRQS1 |= 1 << 29;     // for second level interrupt controller’s
  *AUX_MU_BAUD = 270;    // 115200 baud, system clock freq = 250MHz
  *AUX_MU_IIR = 0x06;    // no FIFO
  *AUX_MU_CNTL = 3;      // enable Tx, Rx
}

void uart_send(unsigned int c) {
  
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x20)); // wait until we can send

  *AUX_MU_IO = c; // write the character to the buffer
}

char uart_getc() {
  do {
    asm volatile("nop");
  } while (!(*AUX_MU_LSR & 0x01));  // wait until something is in the buffer
  
  char r = (char)(*AUX_MU_IO);  // read it and return
  
  return r == '\r' ? '\n' : r;  // convert carrige return to newline
}

void uart_puts(char *s) {
  while (*s) {
    if (*s == '\n')
      uart_send('\r');
    uart_send(*s++);
  }
}

void uart_hex(unsigned int d) {
  for (int c = 28; c >= 0; c -= 4) {
    // get highest tetrad
    unsigned int n = (d >> c) & 0xF;
    // 0-9 => '0'-'9', 10-15 => 'A'-'F'
    n += n > 9 ? 0x37 : 0x30;
    uart_send(n);
  }
}

char uart_getc_pure() {
  /* wait until something is in the buffer */
  do {
    asm volatile("nop");
  } while(!(*AUX_MU_LSR & 0x01));
  /* read it and return */
  return (char)(*AUX_MU_IO);
}

char uart_tx_buffer[MAX_BUF_SIZE] = {};
uint32_t uart_tx_buffer_widx = 0; // write index
uint32_t uart_tx_buffer_ridx = 0; // read index
char uart_rx_buffer[MAX_BUF_SIZE] = {};
uint32_t uart_rx_buffer_widx = 0;
uint32_t uart_rx_buffer_ridx = 0;

void uart_interrupt_r_handler() {
  if ((uart_rx_buffer_widx + 1) % MAX_BUF_SIZE == uart_rx_buffer_ridx) { // buffer full
    disable_uart_r_interrupt();
    return;
  }
  // uart_rx_buffer[uart_rx_buffer_widx++] = uart_getc();
  char r = uart_getc();
  uart_rx_buffer[uart_rx_buffer_widx++] = r == '\r' ? '\n' : r;
  if (uart_rx_buffer_widx >= MAX_BUF_SIZE)
    uart_rx_buffer_widx = 0;
  enable_uart_r_interrupt();
}

void uart_interrupt_w_handler() { //can write
  if (uart_tx_buffer_ridx == uart_tx_buffer_widx) { // buffer empty
    disable_uart_r_interrupt();
    return;
  }
  uart_send(uart_tx_buffer[uart_tx_buffer_ridx++]);
  // *AUX_MU_IO = uart_tx_buffer[uart_tx_buffer_ridx++];
  if (uart_tx_buffer_ridx >= MAX_BUF_SIZE)
    uart_tx_buffer_ridx = 0; // cycle pointer
  enable_uart_w_interrupt();
}

void async_uart_send(char c) {
  uart_tx_buffer[uart_tx_buffer_widx++] = c;
  if (uart_tx_buffer_widx >= MAX_BUF_SIZE)
    uart_tx_buffer_widx = 0;
  enable_uart_w_interrupt();
}

void async_uart_puts(char *s) {
  while (*s) {
    if (*s == '\n')
      async_uart_send('\r');
    async_uart_send(*s++);
  }
}

char async_uart_getc() {
  char res = 0;
  if (uart_rx_buffer_ridx != uart_rx_buffer_widx) { // empty
    res = uart_rx_buffer[uart_rx_buffer_ridx++];
    if (uart_rx_buffer_ridx >= MAX_BUF_SIZE)
      uart_rx_buffer_ridx = 0;
  }
  enable_uart_w_interrupt();
  return res;
}

void enable_uart_r_interrupt(){
  *AUX_MU_IER |= (1);  
}
void enable_uart_w_interrupt(){
  *AUX_MU_IER |= (2);
}

void disable_uart_r_interrupt(){
  *AUX_MU_IER &= ~(1);  
}
void disable_uart_w_interrupt(){
  *AUX_MU_IER &= ~(2);  
}