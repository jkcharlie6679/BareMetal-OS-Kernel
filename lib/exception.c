#include "printf.h"
#include "exception.h"
#include "timer.h"
#include "uart.h"
#include "scheduler.h"
#include "cpio.h"
#include "mailbox.h"
#include "system.h"
#include "vfs.h"
#include "string.h"
#include "malloc.h"

static void signal_handler_wrapper();
static handler_func _handler = NULL;
static uint32_t signal_exit = 0;
static uint64_t signal_pid = 0;
static int sysc_pid();
static int sysc_uart_read(char buf[], size_t size);
static int sysc_uart_write(char buf[], size_t size);
static int sysc_exec(const char* name, char *const argv[], trap_frame *frame);
static int sysc_fork(trap_frame *frame);
static int sysc_exit();
static int sysc_mbox_call(unsigned char ch, unsigned int *mbox);
static int sysc_kill(int pid);
static int sysc_register(int SIGNAL, void (*handler)());
static int sysc_signal_kill(int pid,int SIGNAL);
static int sysc_open(const char *pathname, int flags);
static int sysc_close(int fd);
static int sysc_write(int fd, const void *buf, unsigned long count);
static int sysc_read(int fd, void *buf, unsigned long count);
static int sysc_mkdir(const char *pathname, unsigned mode);
static int sysc_mount(const char* target, const char* file_name);
static int sysc_chdir(const char *path);

void invalid_exception_router(uint64_t x0){
  uint64_t elr_el1, esr_el1, spsr_el1;
  asm volatile("mrs %[output0], elr_el1  \n" :[output0] "=r" (elr_el1) );
  asm volatile("mrs %[output1], ESR_EL1  \n" :[output1] "=r" (esr_el1) );
  asm volatile("mrs %[output2], SPSR_EL1 \n" :[output2] "=r" (spsr_el1));  
  printf("exception number: 0x%x\r\n",x0);
  printf("elr_el1: 0x%x\r\n", elr_el1);
  printf("esr_el1: 0x%x\r\n", esr_el1);
  printf("spsr_el1: 0x%x\r\n", spsr_el1);
  while(1);
}

void irq_router(uint64_t x0){
  if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ){
    core_timer_interrupt_disable();
    pop_timer();
  }else if(*IRQS1_PENDING & (0x01 << 29)){
    if (*AUX_MU_IIR & (0b01 << 1)) {  //can write
      disable_uart_w_interrupt();
      uart_interrupt_w_handler();
    }else if (*AUX_MU_IIR & (0b10 << 1)) {  //can read
      disable_uart_r_interrupt();
      uart_interrupt_r_handler();
    }
  }
}

void sync_router(uint64_t x0, uint64_t x1){
  trap_frame *frame = (trap_frame *)x1;
  int syscall_num = frame->x8;
  // if(syscall_num > 10)
  //   printf("%d", syscall_num);
  switch (syscall_num){
  case SYSCALL_NUM_PID:
    frame->x0 = sysc_pid();
    break;
  case SYSCALL_NUM_UART_READ:
    interrupt_enable();
    frame->x0 = sysc_uart_read((char *)frame->x0, frame->x1);
    interrupt_disable();
    break;
  case SYSCALL_NUM_UART_WRITE:
    interrupt_enable();
    frame->x0 = sysc_uart_write((char *)frame->x0, frame->x1);
    interrupt_disable();
    break;
  case SYSCALL_NUM_EXEC:
    frame->x0 = sysc_exec((char *)frame->x0, (char **) frame->x1, frame);
    break;
  case SYSCALL_NUM_FORK:
    frame->x0 = sysc_fork(frame);
    break;
  case SYSCALL_NUM_EXIT:
    frame->x0 = sysc_exit();
    schedule();
    break;
  case SYSCALL_NUM_MBOX_CALL:
    frame->x0 = sysc_mbox_call((unsigned char)frame->x0, (uint32_t *)frame->x1);
    break;
  case SYSCALL_NUM_KILL:
    frame->x0 = sysc_kill(frame->x0);
    break;
  case SYSCALL_NUM_REGISTER:
    frame->x0 = sysc_register(frame->x0, (void (*)())frame->x1);
    break;
  case SYSCALL_NUM_SIGNAL_KILL:
    frame->x0 = sysc_signal_kill(frame->x0, frame->x1);
    break;
  case SYSCALL_NUM_OPEN:
    frame->x0 = sysc_open((const char *)frame->x0, frame->x1);
    break;
  case SYSCALL_NUM_CLOSE:
    frame->x0 = sysc_close(frame->x0);
    break;
  case SYSCALL_NUM_WRITE:
    frame->x0 = sysc_write(frame->x0, (const void*)frame->x1, (uint64_t)frame->x2);
    break;
  case SYSCALL_NUM_READ:
    frame->x0 = sysc_read(frame->x0, (void *)frame->x1, (uint64_t)frame->x2);
    break;
  case SYSCALL_NUM_MKDIR:
    frame->x0 = sysc_mkdir((const char *)frame->x0, frame->x1);;
    break;
  case SYSCALL_NUM_MOUNT:
    frame->x0 = sysc_mount((const char *)frame->x1, (const char *)frame->x2);
    break;
  case SYSCALL_NUM_CHDIR:
    frame->x0 = sysc_chdir((const char *)frame->x0);
    break;
  default:
    if(x0 == 8)
      printf("syscall number: %d\n\r", syscall_num);
    invalid_exception_router(x0);
    break;
  }
}

static int sysc_pid(){
  task *cur = get_current();
  return cur->pid;
}

static int sysc_uart_read(char buf[], size_t size){
  for(int i=0; i < size; i++)
    buf[i] = uart_getc();
  return size;
}

static int sysc_uart_write(char buf[], size_t size){
  for(int i=0; i<size; i++)
    uart_send(buf[i]);
  return size;
}

static int sysc_exec(const char* name, char *const argv[], trap_frame *frame){
  task *cur = get_current();
  frame->sp_el0 = cur->user_sp + THREAD_SP_SIZE - cur->user_sp%16;
  file *file_handler = NULL;
  char *load_addr = (char *)page_allocate_addr(0x1000 * 64);
  if(vfs_open(name, 0, &file_handler) == 0){
    file_handler->f_ops->read(file_handler, load_addr, 0x1000*64);
    file_handler->f_ops->close(file_handler);
  }
  // char *load_addr = load_program((char *)name);
  frame->elr_el1 = (uint64_t)load_addr;
  return 0;
}

static int sysc_fork(trap_frame *frame){
  task *parent = get_current();
  task *child = task_create(NULL, USER);
  /* copy the task context & kernel stack (including trap frame) of parent to child */
  child->x19 = frame->x19;
  child->x20 = frame->x20;
  child->x21 = frame->x21;
  child->x22 = frame->x22;
  child->x23 = frame->x23;
  child->x24 = frame->x24;
  child->x25 = frame->x25;
  child->x26 = frame->x26;
  child->x27 = frame->x27;
  child->x28 = frame->x28;
  child->fp = frame->x29;
  child->lr = (uint64_t)child_return_from_fork;
  child->sp = (uint64_t)frame;
  child->target_func = parent->target_func;
  child->handler = parent->handler;
  // copy the stack
  char *src1 = (char *)parent->user_sp;
  char *dst1 = (char *)child->user_sp;
  char *src2 = (char *)parent->sp_addr;
  char *dst2 = (char *)child->sp_addr;
  for(int i=0; i<THREAD_SP_SIZE; i++){
    *(dst1+i) = *(src1+i);
    *(dst2+i) = *(src2+i); 
  }
  if((uint64_t)child->sp_addr > (uint64_t)parent->sp_addr){
    child->sp += ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);
    // child->fp += ((uint64_t)child->sp_addr - (uint64_t)parent->sp_addr);      // fp is the chain this only move the fist element
  }else if((uint64_t)child->sp_addr < (uint64_t)parent->sp_addr){
    child->sp -= ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
    // child->fp -= ((uint64_t)parent->sp_addr - (uint64_t)child->sp_addr);
  }
  trap_frame *child_frame = (trap_frame *)child->sp;
  child_frame->x0 = 0;
  child_frame->x29 = child->fp;
  if((uint64_t)child->user_sp > (uint64_t)parent->user_sp){
    child_frame->sp_el0 += ((uint64_t)child->user_sp - (uint64_t)parent->user_sp);
  }else if((uint64_t)child->user_sp < (uint64_t)parent->user_sp){
    child_frame->sp_el0 -= ((uint64_t)parent->user_sp - (uint64_t)child->user_sp);
  }
  return child->pid;
}

static int sysc_exit(){
  task *cur = get_current();
  if(signal_exit){
    signal_exit = 0;
    sys_kill(signal_pid);
  }
  cur->state = EXIT;
  return 0;
}

static int sysc_mbox_call(unsigned char ch, unsigned int *mbox){
  return mbox_call(ch, mbox);
}

static int sysc_kill(int pid){
  kill_thread(pid);
  return 0;
}

static int sysc_register(int SIGNAL, void (*handler)()){
  task *cur = get_current();
  cur->handler = handler;
  return 0;
}

static int sysc_signal_kill(int pid,int SIGNAL){
  task *target = find_task(pid);
  _handler = (handler_func)target->handler;
  signal_pid = pid;
  signal_exit = 1;
  remove_task(pid);
  task *handler_task = task_create(NULL, USER);
  handler_task->target_func = (uint64_t)signal_handler_wrapper;
  return 0;
}

static int sysc_open(const char *pathname, int flags){
  task *cur = get_current();
  int fd = get_task_idle_fd(cur);
  if(fd < 0){
    printf("[ERROR][sys_open] find idle fd\n\r");
    return -1;
  }
  char abs_path[TMPFS_MAX_PATH_LEN];
  abs_path[0] = '\0';
  to_abs_path(abs_path, cur->cwd, pathname);
  // printf("open %s %d\n\r", abs_path, flags);
  file *open_file = NULL;
  vfs_open((const char *)abs_path, flags, &open_file);
  cur->fd_table[fd] = open_file;
  return fd;
}

static int sysc_close(int fd){
  task *cur = get_current();
  file *target_file = cur->fd_table[fd];
  int result = vfs_close(target_file);
  if(result == 0){
    cur->fd_table[fd] = 0;
  }
  return 0;
}

static int sysc_write(int fd, const void *buf, unsigned long count){
  task *cur = get_current();
  file *target_file = cur->fd_table[fd];
  // printf("write %s\n\r", target_file->vnode->component->name);
  return vfs_write(target_file, buf, count);
}

static int sysc_read(int fd, void *buf, unsigned long count){
  task *cur = get_current();
  file *target_file = cur->fd_table[fd];
  // printf("read %s\n\r", target_file->vnode->component->name);
  return vfs_read(target_file, buf, count);
}

static int sysc_mkdir(const char *pathname, unsigned mode){
  task *cur = get_current();
  char abs_path[TMPFS_MAX_PATH_LEN];
  abs_path[0] = '\0';
  to_abs_path(abs_path, cur->cwd, pathname);
  vfs_mkdir((const char *)abs_path);
  // printf("mkdir %s\n\r", abs_path);
  return 0;
}

static int sysc_mount(const char* target, const char* file_name){
  task *cur = get_current();
  char abs_path[TMPFS_MAX_PATH_LEN];
  abs_path[0] = '\0';
  to_abs_path(abs_path, cur->cwd, target);
  // printf("mount %s\n\r", abs_path);
  return vfs_mount((const char *)abs_path, file_name);
}

static int sysc_chdir(const char *path){
  task *cur = get_current();
  char changed_path[TMPFS_MAX_PATH_LEN];
  changed_path[0] = '\0';
  to_abs_path(changed_path, cur->cwd, path);
  vnode *node = NULL;
  int ret = vfs_lookup(changed_path, &node);
  if(changed_path[strlen(changed_path)-1] != '/')
    strcat_(changed_path, "/");
  if(ret == 0)
    strcpy(cur->cwd, changed_path);
  // printf("cd %s\n\r", changed_path);
  return ret;
}

void signal_handler_wrapper(){
  if (_handler){
    _handler();
    add_to_queue();
  }else{
    sys_kill(signal_pid);  // no register kill thread
  }
  signal_exit = 0;
  sys_exit();
}
