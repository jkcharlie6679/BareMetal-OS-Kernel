#include "stdint.h"
#include "stddef.h"

int sys_getpid();
size_t sys_uart_read(char buf[], size_t size);
size_t sys_uart_write(const char buf[], size_t size);
int sys_exec(const char* name, char *const argv[]);
int sys_fork();
void sys_exit();
int sys_mbox_call(unsigned char ch, unsigned int *m_box);
void sys_kill(int pid);
void sys_signal(int SIGNAL, void (*handler)());

