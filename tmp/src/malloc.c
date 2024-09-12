#include "malloc.h"
#include "printf.h"
#include "cpio.h"
#include "fdtb.h"

extern unsigned long _kernel_start;
extern unsigned long _kernel_end;
extern unsigned char _heap_start;

typedef struct pool_header {
  uint16_t chunck_size;
  uint16_t total;
  uint16_t used;
  uint16_t shifting;  // for align 16
  struct pool_header *next;
} pool_header;

static char* heap_top = NULL;
page_info *pages = NULL;
frame_info *free_frame_list[MAX_FRAME_ORDER + 1] = {0}; // linked-list for free page
pool_header *pool = NULL;

// static void pages_state() {
//   printf("--------------pages state--------------\n\r");
//   for (int i = 0; i < PAGE_MAX_ENTRY; i++) {
//     printf("page[%2d] = %2d, ", i, pages[i].status);
//     if (i % 4 == 3 || i == PAGE_MAX_ENTRY - 1)
//       printf("\n\r");
//   }
//   printf("--------------pages state--------------\n\r");
// }

// static void frame_list_state() {
//   printf("--------------free frame list--------------\n\r");
//   frame_info *cur;
//   for (int i = 0; i < MAX_FRAME_ORDER; i++) {
//     cur = free_frame_list[i];
//     printf("list%d: ", i);
//     while (cur != NULL) {
//       printf("%d ", cur->index);
//       cur = cur->next;
//     }
//     printf("\n\r");
//   }
//   printf("--------------free frame list--------------\n\r");
// }

static void* simple_malloc(size_t size) {
  if (size % 0x10)
    size = 0x10 + size - size % 0x10;  // ALIGN 16
  char *return_ptr = heap_top;
  heap_top += size;
  return return_ptr;
}

static int log2(int value) {
  int ans = 1;
  for (int i = 0; ;i++) {
    if(ans > value)
      return --i;
    ans *= 2;
  }
}

static int power2(int value) {
  int ans = 1;
  for (int i = 0; i < value; i++)
    ans *= 2;
  return ans;
}

static void add_to_frame_list(int level, int index) {
  frame_info *item = (frame_info *)((uint64_t)index * PAGE_SIZE);
  item->index = index;
  item->next = NULL;

  if (level < MAX_FRAME_ORDER && level >= 0) {
    if (!free_frame_list[level]) {
      free_frame_list[level] = item;
    } else {
      item->next = free_frame_list[level];
      free_frame_list[level] = item;
    }
  }
}

static void delete_from_frame_list(int level, int idx) {
  frame_info *pre, *cur;
  cur = free_frame_list[level];
  pre = NULL;
  while (cur && cur->index != idx) {
    pre = cur;
    cur = cur->next;
  }
  if (pre != NULL)
    pre->next = cur->next;
  else
    free_frame_list[level] = free_frame_list[level]->next;  // remove head
}

static int devide_frame(int level) {
  if (level > MAX_FRAME_ORDER)
    return -1;
  if (free_frame_list[level] == NULL)
    devide_frame(level + 1);
  if (free_frame_list[level]) {
    int idx = free_frame_list[level]->index;
    free_frame_list[level] = free_frame_list[level]->next; // pop from free list
    pages[idx].status = level - 1; // set the frame size as half
    pages[idx + power2(level - 1)].status= level - 1;
    add_to_frame_list(level - 1, idx + power2(level - 1));  // put the buttom back to list, for can get the first when allocate
    add_to_frame_list(level - 1, idx);
    return 0;
  }
  return -1; // fail to devide
}

static int get_free_frame(int level) {
  int idx = -1;
  if (level < MAX_FRAME_ORDER && level >= 0) {
    for (int i = 0; i < power2(level); i++)
      pages[free_frame_list[level]->index + i].status = PAGE_ALLOCATED;
    idx = free_frame_list[level]->index;
    free_frame_list[level] = free_frame_list[level]->next;
  }
  return idx;
}

static int frame_allocate(uint32_t level) {  // size: number of page will be power of 2
  if (!free_frame_list[level]) {
    if (level < MAX_FRAME_ORDER - 1)
      devide_frame(level + 1);
    else {
      printf("Request too LARGE size!!\n\r");
      return -1;
    }
  }
  int idx = get_free_frame(level);
  pages[idx].idx = level;
  return idx;
}

static void merge_frame(int idx) {
  int level = pages[idx].status;
  int size = power2(pages[idx].status);  // pages size
  int merge_idx = idx + size;     // merge_idx need to be the second half
  if ((idx + size) % (2 * size) == 0) { // idx from funxtion parameter is second half
    int tmp = idx;
    idx = idx - size;
    merge_idx = tmp;
  }
  if (pages[idx].status != pages[merge_idx].status) // check both can be merged
    return;
  delete_from_frame_list(level, idx);
  delete_from_frame_list(level, merge_idx);
  add_to_frame_list(level + 1, idx);
  pages[idx].status = level + 1;
  pages[merge_idx].status = PAGE_NOT_ALLOCATE;
  merge_frame(idx);
}

static void free_frame(int idx) {
  int level = pages[idx].idx;
  add_to_frame_list(level, idx);
  pages[idx].status = level;
  for (int i = 1; i < power2(level); i++)
    pages[idx + i].status = PAGE_NOT_ALLOCATE;
  merge_frame(idx);
}

static void *open_frame_to_pool(size_t size) {
  int num_pages = ((size + sizeof(pool_header) + 1 + 0x10) - 1) / PAGE_SIZE + 1;  // 0x10 for shifting to align 16
  int level = 1;
  if (num_pages > 1)
    level = log2(size - 1) + 1;
  uint64_t page_index = frame_allocate(level);

  pool_header *page = (pool_header *)(PAGE_SIZE * page_index);

  page->chunck_size = size;
  page->total = (PAGE_SIZE - sizeof(pool_header) - 0x10) / (size + 1); // one for record the chunck used or not
  page->used = 1;
  page->next = pool;
  page->shifting = 0;
  if (page->total % 0x10)
    page->shifting = 0x10 - page->total % 0x10;
  pool = page;
  uint8_t *chunck_info = (uint8_t *)(((uint64_t)page) + sizeof(pool_header));
  chunck_info[0] = 1;
  for (int i = 1; i < page->total; i++)
    chunck_info[i] = 0;
  return (void *)(PAGE_SIZE * page_index + sizeof(pool_header) + page->total + page->shifting);
}

void *malloc(size_t size) {
  if (size < 8)
    size = 8;
  int size_level = log2(size - 1) + 1;
  if (size_level > 11)  // TODO: free can't find the page header
    return 0;
  size = power2(size_level);  // align the memory chunck size
  pool_header *cur = pool;
  while (cur) {
    if (cur->chunck_size != size || cur->used >= cur->total)
      cur = cur->next;
    else {
      cur->used += 1;
      uint8_t *page_info = (uint8_t *)((uint64_t)cur + sizeof(pool_header));
      for (int i = 0; i < cur->total; i++) {
        if (page_info[i] == 0) {
          page_info[i] = 1;
          return (void *)((uint64_t)cur + sizeof(pool_header) + cur->total + cur->shifting + cur->chunck_size * i);
        }
      }
    }
  }
  return open_frame_to_pool(size);
}

static void remove_page_from_pool(void *addr) {
  int idx = (uint64_t)(addr) / PAGE_SIZE;
  pool_header *cur = pool;
  pool_header *pre = NULL;
  while (cur != addr) {
    pre = cur;
    cur = cur->next;
  }
  if (pre == NULL)
    pool = cur->next;
  else
    pre->next = cur->next;
  free_frame(idx);
}

void free(void *addr) {
  pool_header *page = (pool_header *)((uint64_t)addr - ((uint64_t)(addr) % PAGE_SIZE));  // find the pool_header addr
  int index = (((uint64_t)(addr) - page->total - page->shifting - sizeof(pool_header)) % PAGE_SIZE) / page->chunck_size;
  if (page->used != 1) {
    page->used -= 1;
    uint8_t *page_info = (uint8_t *)(addr + sizeof(pool_header));
    page_info[index] = 0;
  } else {
    remove_page_from_pool(addr);
  }
}

static void memory_reserve(uint64_t start, uint64_t end) {
  int start_idx = start / PAGE_SIZE;
  int end_idx = end / PAGE_SIZE;
  
  while (start_idx <= end_idx && pages[start_idx].status == PAGE_ALLOCATED)
    start_idx++;
  while (start_idx <= end_idx && pages[end_idx].status == PAGE_ALLOCATED)
    end_idx--;
  if (start_idx > end_idx)
    return;
  
  // printf("reserve from page %d to page %d\n\r", start_idx, end_idx);

  int head = start_idx ;
  while (pages[head].status == PAGE_NOT_ALLOCATE)
    head--;

  while (pages[head].status != PAGE_ALLOCATED) {
    if (head == start_idx) {
      pages[head].idx = pages[head].status;
      for (int i = 0; i < power2(pages[head].idx); ++i)
        pages[head + i].status = PAGE_ALLOCATED;
      break;
    }
    pages[head].status = pages[head].status - 1;
    pages[head + power2(pages[head].status)].status = pages[head].status;
    int half = head + power2(pages[head].status);
    if (half <= start_idx) {
      head += power2(pages[head].status);
    } 
  }

  while (head + power2(pages[head].idx) - 1 > end_idx) {
    pages[head].idx--;
    int half = head + power2(pages[head].idx);
    pages[half].status = pages[head].idx;
    for (int i = 1; i < power2(pages[half].status); ++i)
      pages[half + i].status = PAGE_NOT_ALLOCATE;
  }

  if (head + power2(pages[head].idx) - 1 == end_idx)
    return;

  head = end_idx;
  while (pages[head].status == PAGE_NOT_ALLOCATE)
    head--;


  while (pages[end_idx].status != PAGE_ALLOCATED) {
    if (head + power2(pages[head].status) - 1 == end_idx) {
      pages[head].idx = pages[head].status;
      for (int i = 0; i < power2(pages[head].idx); ++i)
        pages[head + i].status = PAGE_ALLOCATED;
      break;
    }

    pages[head].status = pages[head].status - 1;
    int half = head + power2(pages[head].status);
    pages[half].status = pages[head].status;
    if (half - 1 < end_idx) { // end_idx at the second half
      head += power2(pages[head].status);
    } 
  }

  for (int i = start_idx; i <= end_idx; i++) {
    if (pages[i].status == PAGE_NOT_ALLOCATE)
      pages[i].status = PAGE_ALLOCATED;
    if (pages[i].status != PAGE_NOT_ALLOCATE && pages[i].status != PAGE_ALLOCATED) {
      pages[i].idx = pages[i].status;
      pages[i].status = PAGE_ALLOCATED;
    }
  }
}

void page_init(void) {
  heap_top = (char*) ((uint64_t)&_heap_start + PAGE_SIZE - ((uint64_t)&_heap_start % PAGE_SIZE));  // prevent for cover to the kernel and let the heap align the 0x1000 (page size)

  pages = (page_info *)simple_malloc(PAGE_MAX_ENTRY * sizeof(page_info));
  char *sim_alloc_start = (char *)pages;
  char *sim_alloc_end = sim_alloc_start + PAGE_MAX_ENTRY * sizeof(frame_info);

  int non_init = PAGE_MAX_ENTRY;
  for (int start = 0; start < PAGE_MAX_ENTRY; ) {
    pages[start].status = log2(non_init);
    pages[start].idx = PAGE_NOT_ALLOCATE;

    start += power2(log2(non_init));
    for (int i = PAGE_MAX_ENTRY - non_init + 1; i < start; i++) {
      pages[i].status = PAGE_NOT_ALLOCATE;
      pages[i].idx = PAGE_NOT_ALLOCATE;
    }

    non_init -= power2(log2(non_init));
  }

  memory_reserve((uint64_t)SPIN_TABLE_START, (uint64_t)SPIN_TABLE_END);       // spin table
  memory_reserve((uint64_t)&_kernel_start, (uint64_t)&_kernel_end);           // kernel
  memory_reserve((uint64_t)sim_alloc_start, (uint64_t)sim_alloc_end);         // simple allocate for the page management
  memory_reserve((uint64_t)CPIO_DEFAULT_PLACE, (uint64_t)CPIO_DEFAULT_END);   // cpio size
  memory_reserve((uint64_t)fdtb_place, (uint64_t)fdtb_place + fdtb_size);     // dtbmak

  for (int head = 0; head < PAGE_MAX_ENTRY; ) {
    if (pages[head].status != PAGE_NOT_ALLOCATE && pages[head].status != PAGE_ALLOCATED) {
      add_to_frame_list(pages[head].status, head);
    }
    head += power2(pages[head].idx);
  }

}