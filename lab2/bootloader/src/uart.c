#include "gpio.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))


void uart_init()
{
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
  *AUX_MU_IER = 0;       // disable interrupt
  *AUX_MU_LCR = 3;       // set 8 bits
  *AUX_MU_MCR = 0;
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
