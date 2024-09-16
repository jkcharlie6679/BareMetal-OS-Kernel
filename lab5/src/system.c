#include "system.h"
#include "scheduler.h"

int sys_getpid(){
  asm volatile("mov x8, 0\n");
  asm volatile("svc 0\n\t");
  register int pid asm("x0");
  return pid;
}

size_t sys_uart_read(char buf[], size_t size){
  asm volatile("mov x0, %[output]\n"::[output]"r"(buf));
  asm volatile("mov x1, %[output]\n"::[output]"r"(size));
  asm volatile("mov x8, 1\n");
  asm volatile("svc 0\n");
  register int read asm("x0");
  return read;
}


size_t sys_uart_write(const char buf[], size_t size){
  asm volatile("mov x0, %[output]\n"::[output]"r"(buf));
  asm volatile("mov x1, %[output]\n"::[output]"r"(size));
  asm volatile("mov x8, 2\n");
  asm volatile("svc 0\n");
  register int sent asm("x0");
  return sent;
}

int sys_exec(const char* name, char *const argv[]){
  asm volatile("mov x0, %[output]\n"::[output]"r"(name));
  asm volatile("mov x1, %[output]\n"::[output]"r"(argv));
  asm volatile("mov x8, 3\n");
  asm volatile("svc 0\n");
  register int value asm("x0");
  return value;
}

int sys_fork(){
  asm volatile("mov x8, 4\n");
  asm volatile("svc 0\n\t");
  register int value asm("x0");
  return value;
}

void sys_exit(){
  asm volatile("mov x8, 5\n");
  asm volatile("svc 0\n\t");
}

int sys_mbox_call(unsigned char ch, unsigned int *m_box){
  asm volatile("mov x0, %[output]\n"::[output]"r"(ch));
  asm volatile("mov x1, %[output]\n"::[output]"r"(m_box));
  asm volatile("mov x8, 6\n");
  asm volatile("svc 0\n");
  register int value asm("x0"); 
  return value;
}

void sys_kill(int pid){
  asm volatile("mov x0, %[output]\n"::[output]"r"(pid));
  asm volatile("mov x8, 7\n");
  asm volatile("svc 0\n");
}

void sys_signal(int SIGNAL, void (*handler)()){
  asm volatile("mov x0, %[output]\n"::[output]"r"(SIGNAL));
  asm volatile("mov x1, %[output]\n"::[output]"r"(handler));
  asm volatile("mov x8, 8\n");
  asm volatile("svc 0\n");
}
