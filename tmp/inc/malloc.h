#ifndef MALLOC_H
#define MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define PAGE_MAX_ENTRY 56
#define MAX_CONTINUE_PAGE 32
#define MAX_FRAME_POWER   6
#define PAGE_SIZE 0x1000

#define PAGE_NOT_ALLOCATE -1  // for the frame array which is not allocated
#define PAGE_ALLOCATED    -2  // for the frame array is allocated

typedef uint64_t size_t;

typedef struct malloc_header {
  unsigned int previous;
  unsigned int chunk;
} malloc_header;

typedef struct free_frame {
  uint32_t index;
  // uint32_t queue_index;
  struct free_frame *next;
} free_frame;

void* simple_malloc(size_t size);
void page_init(void);
int page_allocate(size_t size);
void page_free(int idx);
void *malloc(size_t size);

#ifdef __cplusplus
}
#endif

#endif