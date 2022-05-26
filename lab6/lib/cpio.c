#include "cpio.h"
#include "printf.h"
#include "string.h"
#include "malloc.h"
#include "mmu.h"

static int read_header(char *str, int size);

char *CPIO_DEFAULT_PLACE;
char *CPIO_DEFAULT_PLACE_END;

void cpio_ls(){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  while(strcmp(header+110, "TRAILER!!!")){
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }
    printf("%s\n\r", header+110);
    header = header + namesize + filesize + 110;
  }
}

void cpio_cat(char *str){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  while(strcmp(header+110, "TRAILER!!!")){
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    namesize = namesize + namesize%2;
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }
    if(!strcmp(header+110, str) && filesize != 0){
      printf("%s\n\r", header+110+namesize);
      return;
    }
    header = header + namesize + filesize + 110;
  }
  printf("can't find this file named \"%s\"\n\r", str);
}

void cpio_exec(char *str){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  char *data;
  while(strcmp(header+110, "TRAILER!!!")){
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    namesize = namesize + namesize%2;
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }
    if(!strcmp(header+110, str) && filesize != 0){
      data = header+110+namesize;
      char * sp = malloc(0x20);
      asm volatile("mov x1        , 0x3c0     \n");
      asm volatile("msr spsr_el1  , x1        \n");
      asm volatile("msr elr_el1   , %[input0] \n"::[input0]"r"(data));
      asm volatile("msr sp_el0    , %[input1] \n"::[input1]"r"(sp));
      asm volatile("eret\n");
    }
    header = header + namesize + filesize + 110;
  }
  printf("can't find this file named \"%s\"\n\r", str);
}

int read_header(char *str, int size){
  char a[size + 1];
  a[1] = 0;
  for(int i=0; i<size; i++){
    append_str(a, *(str+i));
  }
  return myHex2Int(a);
}

void load_program(char *name, uint64_t *page_table){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = (char *)PA2VA(cpio->c_magic);
  char *data;
  while(strcmp(header+110, "TRAILER!!!")){
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    namesize = namesize + namesize%2;
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }

    if(!strcmp(header+110, name) && filesize != 0){
      data = header+110+namesize;
      int sz = filesize / 0x1000 + (filesize % 0x1000 != 0);
      for (int i = 0; i < sz; i++) {
        char *addr = (char *)page_allocate_addr(0x1000);
        map_pages(page_table, USER_PROGRAM_ADDR + 0x1000 * i, (uint64_t)VA2PA(addr));
        for (int j = 0; j < 0x1000; j++){
          *(addr+j) = *(data+i*0x1000+j);
        }
      }
      return;
    }
    header = header + namesize + filesize + 110;
  }
  printf("can't find this file named \"%s\"\n\r", name);
  return;
}
