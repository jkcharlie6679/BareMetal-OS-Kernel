#include "malloc.h"
#include "printf.h"

extern unsigned char _heap_start;
static char* top = (char*) &_heap_start;

void* simple_malloc(size_t size) {
  char* r = top + 0x10;  // reserve for header
  size = 0x10 + size - size % 0x10;  // ALIGN 16
  ((malloc_header*)top)->chunk = size;
  top += size + 0x10;
  return r;
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

int pages[PAGE_MAX_ENTRY] = {0}; // array for pages
int pages_allocated[PAGE_MAX_ENTRY] = {0}; // store the allocated frame size (level)

free_frame *free_frame_list[MAX_FRAME_POWER] = {NULL}; // linked-list for free page
free_frame free_item[PAGE_MAX_ENTRY]; // unit ad the page

static void add_frmae_list(int level, int index) {
  free_frame *item = &free_item[index];
  item->index = index;
  item->next = NULL;

  if (level < MAX_FRAME_POWER && level >= 0) {
    if (!free_frame_list[level]) {
      free_frame_list[level] = item;
    } else {
      item->next = free_frame_list[level];
      free_frame_list[level] = item;
    }
  } else
    printf("Out of rnage!!!\n\r");
}

static void pages_state() {
  printf("--------------pages state--------------\n\r");
  for (int i = 0; i < PAGE_MAX_ENTRY; i++) {
    printf("page[%2d] = %2d, ", i, pages[i]);
    if (i % 4 == 3 || i == PAGE_MAX_ENTRY - 1)
      printf("\n\r");
  }
  printf("--------------pages state--------------\n\r");
}

static void frame_list_state() {
  printf("--------------free frame list--------------\n\r");
  free_frame *cur;
  for (int i = 0; i < MAX_FRAME_POWER; i++) {
    cur = free_frame_list[i];
    printf("list%d: ", i);
    while (cur != NULL) {
      printf("%d ", cur->index);
      cur = cur->next;
    }
    printf("\n\r");
  }
  printf("--------------free frame list--------------\n\r");
}

void page_init(void) {
  int non_init = PAGE_MAX_ENTRY;
  for (int start = 0; start < PAGE_MAX_ENTRY; ) {
    pages[start] = log2(non_init);
    add_frmae_list(log2(non_init), start);
    start += power2(log2(non_init));

    for (int i = PAGE_MAX_ENTRY - non_init + 1; i < start; i++)
      pages[i] = PAGE_NOT_ALLOCATE;
    non_init -= power2(log2(non_init));
  }
  pages_state();
  frame_list_state();
}

static int page_devide(int level) {
  if (level > MAX_FRAME_POWER)
    return -1;
  if (free_frame_list[level] == NULL)
    page_devide(level + 1);
  if (free_frame_list[level]) {
    int idx = free_frame_list[level]->index;
    free_frame_list[level] = free_frame_list[level]->next; // pop from free list
    pages[idx] = level - 1; // set the frame size as half
    pages[idx + power2(level - 1)]= level - 1;
    add_frmae_list(level - 1, idx + power2(level - 1));  // put the buttom back to list, for can get the first when allocate
    add_frmae_list(level - 1, idx);
    return 0;
  }
  return -1; // fail to devide
}

static int allocate_frame_list(int value) {
  int idx = -1;
  if (value < MAX_FRAME_POWER && value >= 0) {
    for (int i = 0; i < power2(value); i++)
      pages[free_frame_list[value]->index + i] = PAGE_ALLOCATED;
    idx = free_frame_list[value]->index;
    free_frame_list[value] = free_frame_list[value]->next;
    return idx;
  }
  
  return -1;
}

static void delete_frame_list(int level, int idx) {
  free_frame *pre, *cur;
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

static void merge(int idx) {
  int level = pages[idx];
  int size = power2(pages[idx]);  // pages size
  int merge_idx = idx + size;     // merge_idx need to be the second half
  if ((idx + size) % (2 * size) == 0) { // idx from funxtion parameter is second half
    int tmp = idx;
    idx = idx - size;
    merge_idx = tmp;
  }
  if (pages[idx] != pages[merge_idx]) // check both can be merged
    return;
  delete_frame_list(level, idx);
  delete_frame_list(level, merge_idx);
  add_frmae_list(level + 1, idx);
  pages[idx] = level + 1;
  pages[merge_idx] = PAGE_NOT_ALLOCATE;
  merge(idx);
}

int page_allocate(size_t size) {
  int level = log2(size / PAGE_SIZE);
  if (!free_frame_list[level]) {
    if (level < MAX_FRAME_POWER - 1)
      page_devide(level + 1);
    else {
      printf("Request too LARGE size!!\n\r");
      return -1;
    }
  }
  int idx = allocate_frame_list(level);
  pages_allocated[idx] = level;
  pages_state();
  frame_list_state();
  return idx;
}

void page_free(int idx) {
  int level = pages_allocated[idx];
  pages_allocated[idx] = 0; // reset
  add_frmae_list(level, idx);
  pages[idx] = level;
  for (int i = 1; i < power2(level); i++)
    pages[idx + i] = PAGE_NOT_ALLOCATE;
  merge(idx);
  pages_state();
  frame_list_state();
}
