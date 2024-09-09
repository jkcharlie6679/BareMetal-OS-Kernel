#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_hex(unsigned int d);
char uart_getc_pure();

#ifdef __cplusplus
}
#endif

#endif
