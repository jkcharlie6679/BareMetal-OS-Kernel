#include "dtb.h"
#include "printf.h"
#include "string.h"
#include "cpio.h"

char* dtb_place;
uint32_t dtb_size;

static uint32_t little2big(uint32_t str);

void fdt_traverse(dtb_callback callback){
  fdt_header* header = (fdt_header*) dtb_place;

  if(little2big(header->magic) != 0xD00DFEED){
    printf("fdt_traverse : wrong magic in fdt_traverse\n\r");
    return;
  }

  uint32_t struct_size = little2big(header->size_dt_struct);
  dtb_size = struct_size;
  char* dt_struct_ptr = (char*)((char*)header + little2big(header->off_dt_struct));
  char* dt_strings_ptr = (char*)((char*)header + little2big(header->off_dt_strings));
  char* end = (char*)dt_struct_ptr + struct_size;
  char* pointer = dt_struct_ptr;
  fdt_prod *prod_ptr;

  while(pointer < end){
    uint32_t token_type = little2big(*(uint32_t*)pointer);
    pointer += 4;
    if(token_type == FDT_BEGIN_NODE) { 
      callback(token_type, pointer, 0, 0);
      pointer += strlen(pointer);
      pointer += 4 - (uint64_t)pointer%4; 
    }else if(token_type == FDT_PROP) {
      prod_ptr = (fdt_prod *) pointer;
      pointer += 8;                                 // add the fdt_prod len
      uint32_t len = little2big(prod_ptr->len);
      char* name = (char*)dt_strings_ptr + little2big(prod_ptr->nameoff);
      callback(token_type, name, pointer, len);
      pointer += len;
      if((uint64_t)pointer % 4 !=0){
        pointer += 4 - (uint64_t)pointer%4;
      }
    }
  }
}

void initramfs_callback(uint32_t node_type, char *name, void *value, uint32_t name_size) {
  if(node_type==FDT_PROP && strcmp(name,"linux,initrd-start")==0){
    CPIO_DEFAULT_PLACE = (void *)(uint64_t)little2big(*(uint32_t*)value);
  }
  if(node_type==FDT_PROP && strcmp(name,"linux,initrd-end")==0){
    CPIO_DEFAULT_PLACE_END = (void *)(uint64_t)little2big(*(uint32_t*)value);
  }
}


uint32_t little2big(uint32_t str){
  return  ((str>>24)&0xff) |        // move byte 3 to byte 0
          ((str<<8)&0xff0000) |     // move byte 1 to byte 2
          ((str>>8)&0xff00) |       // move byte 2 to byte 1
          ((str<<24)&0xff000000);   // byte 0 to byte 3
}