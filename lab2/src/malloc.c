#include "malloc.h"

extern unsigned char _heap_start;
static char* top = (char*) &_heap_start;

void* simple_malloc(size_t size) {
  char* r = top + 0x10;  // reserve for header
  size = 0x10 + size - size % 0x10;  // ALIGN 16
  ((malloc_header*)top)->chunk = size;
  top += size + 0x10;
  return r;
}
