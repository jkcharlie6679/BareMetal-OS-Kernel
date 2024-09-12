#ifndef MALLOC_H
#define MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


#define PAGE_NOT_ALLOCATE -1  // for the frame array which is not allocated
#define PAGE_ALLOCATED    -2  // for the frame array is allocated

#define MAX_FRAME_ORDER   15
#define PAGE_SIZE 0x1000

#define PAGE_MAX_ENTRY 0x3c000 // 0x3c000000 / PAGE_SIZE;
#define SPIN_TABLE_START 0x0000
#define SPIN_TABLE_END   0x1000

typedef uint64_t size_t;

typedef struct page_info {
  uint32_t status;  // PAGE_NOT_ALLOCATE || PAGE_ALLOCATED || idx for length
  uint32_t idx;     // idx for free() to know length
} page_info;

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// | pool_header | used or not array (size equal to total) | shifting | chuncks |
// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
typedef struct pool_header {
  uint16_t chunck_size;
  uint16_t total;            // how many chunck can use
  uint16_t used;
  uint16_t shifting;         // for total length (recode the chunck in frame used or not) to align 16
  struct pool_header *next;
} pool_header;

typedef struct frame_info { // the frame put in the free frame list
  uint32_t index;           // store the index of page
  struct frame_info *next;
} frame_info;

void page_init(void);
void *malloc(size_t size);
void free(void *addr);

#ifdef __cplusplus
}
#endif

#endif