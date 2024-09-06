#include "malloc.h"
#include "stdint.h"
#include "printf.h"
#include "cpio.h"
#include "dtb.h"

static void* simple_malloc_(size_t size);
static void add_page_item(int index);
static void delete_page_item(int index);
static void page_divide(int level);
static void memory_reserve(uint64_t start, uint64_t end);
static void merge(int index);
static void *open_page(size_t size);
static void remove_page(void *addr);
static int log2(int value);
static int power2(int value);
static uint64_t aling8(uint64_t value);

static char* heap_top = (char*) &_heap_start + 0x1000;
static unsigned int PAGE_MAX_ENTRY = 0x3c000000/0x1000;
static frame_info *frame;
static page_item *page_list[PAGE_MAX_ORDER+1] = {0};
static pool_header *pool = NULL;

void* simple_malloc_(size_t size) {
  size = 0x10 + size - size%0x10;  // ALIGN 16
  char *return_ptr = heap_top;
  heap_top += size;
  return return_ptr;
}

void *page_allocate_addr(size_t size){
  return (void *)((uint64_t)((page_allocate(size)*0x1000)));
}

void page_init(){
  char *sim_alloc_start = 0;
  char *sim_alloc_end = 0;
  char *addr = simple_malloc_(PAGE_MAX_ENTRY * sizeof(frame_info));
  frame = (frame_info *)addr;
  sim_alloc_start = addr;
  sim_alloc_end = addr + PAGE_MAX_ENTRY * sizeof(frame_info);

  int non_init = PAGE_MAX_ENTRY;
  for(int head=0; head<PAGE_MAX_ENTRY;){
    (frame+head)->index = log2(non_init);
    (frame+head)->status = 0;
    head += power2((frame+head)->index);
    for(uint64_t i=PAGE_MAX_ENTRY-non_init+1; i<head; i++){
      (frame+i)->index = PAGE_FREE;
      (frame+i)->status = 0;
    }
    non_init -= power2(log2(non_init));
  }
  memory_reserve((uint64_t)sim_alloc_start, (uint64_t)sim_alloc_end);                                   // for simple_malloc
  memory_reserve((uint64_t)0x0000, (uint64_t)0x1000);                                                   //spin table
  memory_reserve((uint64_t)&_kernel_start, (uint64_t)&_kernel_end);                                     // kernel
  memory_reserve((uint64_t)&_kernel_start - 0x2000, (uint64_t)&_kernel_start-0x1000);                   // stack
  memory_reserve((uint64_t)CPIO_DEFAULT_PLACE, (uint64_t)CPIO_DEFAULT_PLACE_END+0x1000);                // cpio size
  memory_reserve((uint64_t)dtb_place, (uint64_t)dtb_place+dtb_size);                                    // dtb
  memory_reserve((uint64_t)USER_PROGRAM_SPACE, (uint64_t)USER_PROGRAM_SPACE+USER_PROGRAM_MAX_SIZE);     // user program
  non_init = PAGE_MAX_ENTRY;
  for(int head=0; head<PAGE_MAX_ENTRY;){
    if((frame+head)->status == 0)
      add_page_item(head);
    head += power2((frame+head)->index);
    non_init -= power2(log2(non_init));
  }
}

void add_page_item(int index){
  int level = (frame+index)->index;
  page_item *free_page = (page_item *)(index*0x1000 + 0x1000-sizeof(page_item));
  free_page->index = index;
  free_page->previous = NULL;
  free_page->next = NULL;
  if(!page_list[level]){
    page_list[level] = free_page;
  }else{
    page_item *current = page_list[level];
    free_page->next = page_list[level];
    current->previous = free_page;
    page_list[level] = free_page;
  }
}

void delete_page_item(int index){
  int level = (frame+index)->index;
  page_item *current = (page_item *)(index*0x1000 + 0x1000-sizeof(page_item));
  page_item *previous = current->previous;
  page_item *next = current->next;
  if(next && previous){
    previous->next = next;
    next->previous = previous;
  }else{
    if(next){
      next->previous = NULL;
      page_list[level] = next;
    }else if (previous){
      previous->next = NULL;
    }else{
      page_list[level] = NULL;
    }
  }
}

void memory_reserve(uint64_t start, uint64_t end){
  int start_index = start/0x1000;
  int end_index = end/0x1000;
  int head = start_index ;

  while ((frame+head)->index == -1)
    head--;

  while(head != start_index){
    (frame+head)->index = (frame+head)->index-1;
    (frame+head)->status = 0;
    (frame+head+power2((frame+head)->index))->index = (frame+head)->index;
    (frame+head+power2((frame+head)->index))->status = 0;
    head += power2((frame+head)->index);
    if(head > start_index){
      head -= power2((frame+head)->index);
    }
  }

  while(head+power2((frame+head)->index) < end_index){
    (frame+head)->status = 1;
    for(int i=1; i<power2((frame+head)->index); i++)
      (frame+head+i)->index = PAGE_RESERVED;
    head += power2((frame+head)->index);
  }
  
  while(head+power2((frame+head)->index)-1 != end_index){
    (frame+head)->index = (frame+head)->index-1;
    (frame+head)->status = 0;
    (frame+head+power2((frame+head)->index))->index = (frame+head)->index;
    (frame+head+power2((frame+head)->index))->status = 0;
    if(end_index >= head+power2((frame+head)->index)){
      (frame+head)->status = 1;
      for(int i=1; i<power2((frame+head)->index); i++)
        (frame+head+i)->index = PAGE_RESERVED;
      head += power2((frame+head)->index);
    }
  }
  (frame+head)->status = 1;
  for(int i=1; i<power2((frame+head)->index); i++)
    (frame+head+i)->index = PAGE_RESERVED;
}

int page_allocate(size_t size){
  int level = log2((size-1)/0x1000)+1;
  if(!page_list[level]){
    if(level < PAGE_MAX_ORDER)
      page_divide(level+1);
    else{
      printf("No page to use!!\n\r");
      return -1;
    }
  }
  page_item *current = page_list[level];
  (frame+current->index)->status = 1;
  for(int i=1; i<power2(level); i++)
    (frame+current->index+i)->index = PAGE_ALLOCATED;
  page_list[level] = current->next;
  return current->index;
}

void page_free(int index){
  int size = (frame+index)->index;
  add_page_item(index);
  (frame+index)->status = 0;
  for(int i=1; i<power2(size); i++){
    (frame+index+i)->index = PAGE_FREE;
  }
  merge(index);
}

void merge(int index){
  if((frame+index)->index == PAGE_MAX_ORDER)
   return;
  int size = power2((frame+index)->index);
  int merge_index = index + size;
  if(index%(2*size))
    merge_index = index - size;
  if((frame+index)->index == (frame+merge_index)->index){
    if((frame+index)->status == 0 && (frame+merge_index)->status == 0){
      delete_page_item(index);
      delete_page_item(merge_index);
      if(merge_index > index){
        int tmp;
        tmp = index;
        index = merge_index;
        merge_index = tmp;
      }
      (frame+index)->index = PAGE_FREE;
      (frame+merge_index)->index += 1;
      add_page_item(merge_index);
      merge(merge_index);
      // printf("merge page: %d and page: %d\n\r", index, merge_index);
    }
  }else
    return;
}

void page_divide(int level){
  if(page_list[level]){
    page_item *current = page_list[level];
    int index = current->index;
    page_list[level] = current->next;
    (frame+index)->index -= 1;
    add_page_item(index);
    (frame+index+power2(level-1))->index = level-1;
    add_page_item(index+power2(level-1));
  }else{
    if(level < 5){
      page_divide(level+1);
      page_divide(level);
    }else
      printf("No page can be divided!!!\n\r");
  }
}

void *malloc_(uint64_t size){
  if(size < 8)
    size = 8;
  int sizeLevel = log2(size-1)+1;
  size = power2(sizeLevel);
  if(size > 0x800){
    size *= 2;
  }
  pool_header *cur = pool;
  while (cur){
    if(cur->size < size || cur->size >= 2*size || cur->used >= cur->total)
      cur = cur->next;
    else{
      cur->used += 1;
      uint8_t *page_info = (uint8_t *)((uint64_t)cur + aling8(sizeof(pool_header)));
      for(int i=0; i<cur->total; i++){
        if(page_info[i] == 0){
          page_info[i] = 1;
          return (void *)((uint64_t)cur + aling8(sizeof(pool_header)) + aling8(cur->total) + cur->size*i);
        }
      }
    }
  }
  return open_page(size);
}

void *open_page(size_t size){
  // printf("open %d\n\r", size);
  uint64_t page_index = page_allocate(size);
  pool_header *page = (pool_header *)(0x1000*page_index);
  page->size = size;
  page->total = (0x1000 - aling8(sizeof(pool_header))-16)/(size+1); // sub 8 for aling 8 of use array
  page->used = 1;
  page->next = pool;
  pool = page;
  uint8_t *page_info = (uint8_t *)(0x1000*page_index + aling8(sizeof(pool_header)));
  *page_info = 1;
  for(int i=1; i<page->total; i++)
    *(page_info+i) = 0;
  return (void *)(0x1000*page_index+aling8(sizeof(pool_header))+aling8(page->total));
}

void free(void *addr){
  uint64_t index = (uint64_t)addr % 0x1000;
  addr -= ((uint64_t)addr % 0x1000);
  pool_header *page = (pool_header *)addr;
  index = (index - aling8(page->total) - aling8(sizeof(pool_header)))/page->size;
  if(page->used != 1){
    page->used -= 1;
    uint8_t *page_info = (uint8_t *)(addr + aling8(sizeof(pool_header)));
    page_info[index] = 0;
  }else{
    remove_page(page);
  }
}

void remove_page(void *addr){
  int index = (uint64_t)addr / 0x1000;
  pool_header *cur = pool;
  pool_header *pre = NULL;
  while (cur != addr){
    pre = cur;
    cur = cur->next;
  }
  if(pre == NULL)
    pool = cur->next;
  else
    pre->next = cur->next;
  page_free(index);
}

int log2(int value){
  int ans = 1;
  for(int i=0; ;i++){
    if(ans > value)
      return (--i)>PAGE_MAX_ORDER?PAGE_MAX_ORDER:i; // set the max continue order
    ans *= 2;
  }
}

int power2(int value){
  int ans = 1;
  for(int i=0; i<value; i++)
    ans *= 2;
  return ans;
}

void show_frame(){
  printf("--------------frame state--------------\n\r");
  for(int i=0; i<PAGE_MAX_ENTRY; i++){
    printf("frmae[%2d] = %2d, ", i, (frame+i)->index);
    if(i%4 == 3 || i+1 == PAGE_MAX_ENTRY)
      printf("\n\r");
  }
}

void show_page_list(){
  printf("--------------free frame list--------------\n\r");
  page_item *pre;
  for(int i=0; i<=PAGE_MAX_ORDER; i++){
    pre = page_list[i];

    printf("list %d: ", i);
    while (pre != NULL){
      printf("%d ", pre->index);
      pre = pre->next;
    }
    printf("\n\r");
  }
}

uint64_t aling8(uint64_t value){
  if(value%8 == 0)
    return value;
  return 8+value-value%8;
}
