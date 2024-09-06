#include "uart.h"
#include "shell.h"
#include "printf.h"
#include "malloc.h"
#include "dtb.h"
#include "timer.h"
#include "scheduler.h"
#include "cpio.h"
#include "vfs.h"

void main(char * arg){
  dtb_place = arg;
  uart_init();
  interrupt_enable();
  core_timer_enable();
  fdt_traverse(initramfs_callback);
  page_init();
  vfs_mount("/", "tmpfs");
  vfs_mount("/initramfs", "tmpfs");
  vfs_mount("/dev", "devfs");
  thread_init();
  shell();
}
