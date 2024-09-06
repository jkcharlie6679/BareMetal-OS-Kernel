#include "delay.h"

void delay_tick(uint64_t tick){
  do{
    asm volatile("nop");
  }while(tick--);
}
