#include "cpio.h"
#include "printf.h"
#include "string.h"
#include "malloc.h"
#include "vfs.h"


static int read_header(char *str, int size);
static int mounted = 0;
static void cpio_parse(cpio_file_ll **file);

char * CPIO_DEFAULT_PLACE;
char * CPIO_DEFAULT_PLACE_END;

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
      char * sp = malloc_(0x20);
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

void *load_program(char *name){
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
    if(!strcmp(header+110, name) && filesize != 0){
      data = header+110+namesize;
      // allocate a space and copy the file which need to be executed
      if(filesize > USER_PROGRAM_MAX_SIZE){
        printf("file too large\n\r");
        return 0;
      }
      char *addr = (char *)USER_PROGRAM_SPACE;
      for(int i=0; i<filesize; i++){
        *(addr+i) = *(data+i);
      }
      return addr;
    }
    header = header + namesize + filesize + 110;
  }
  printf("can't find this file named \"%s\"\n\r", name);
  return 0;
}


static void cpio_parse(cpio_file_ll **file){
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  cpio_file_ll *ptr;
  char *header = cpio->c_magic;
  while(strcmp(header+110, "TRAILER!!!")){
    cpio_file_ll *cur = malloc_(sizeof(cpio_file_ll));
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    if((namesize+110)%4){
      namesize += (4 - (namesize+110)%4);
    }
    if(filesize%4){
      filesize += (4 - filesize%4);
    }

    cur->file_size = filesize;
    cur->pathname = (uint8_t *)header+110;
    cur->data_ptr = (uint8_t *)header+110+namesize;
    if(*file == NULL){
      *file = cur;
      ptr = cur;
    }else{
      ptr->next = cur;
      ptr = cur;
    }
    header = header + namesize + filesize + 110;
  }
  ptr->next = NULL;
}

/* For the Virtual File System */

int initramfs_setup_mount(struct filesystem *fs, mount *mount){
  if(mount == NULL){
    printf("Error, initramfs_setup_mount(), NULL pointer.");
  }
  mount->root = malloc_(sizeof(vnode));
  mount->root->mount = NULL;
  
  mount->root->component = malloc_(sizeof(vnode_component));
  mount->root->component->name = "";
  mount->root->component->len = 0;
  mount->root->component->entries = NULL;
  mount->root->component->type = COMP_DIR;
  
  mount->root->f_ops = malloc_(sizeof(file_operations));
  mount->root->f_ops->write = initramfs_write;
  mount->root->f_ops->read = initramfs_read;
  mount->root->f_ops->open = initramfs_open;
  mount->root->f_ops->close = initramfs_close;

  mount->root->v_ops = malloc_(sizeof(vnode_operations));
  mount->root->v_ops->mkdir = initramfs_mkdir;
  mount->root->v_ops->create = initramfs_create;
  mount->root->v_ops->lookup = initramfs_lookup;
  
  if(mounted)
    return 0;

  // Insert files in the linked list to the file system
  cpio_file_ll *file = NULL;

  cpio_parse(&file); 
  
  vnode *dir_node = mount->root;
  vnode *node_new = NULL;
  while(file->next != NULL){
    // Skip 0 files
    if(file->file_size > 0){
      // Create entry
      // printf("%d %s\n\r", file->file_size, file->pathname);
      printf("%");  // TODO add this for some bug I don't know why
      lookup_path((char*)file->pathname, dir_node, &node_new, 1);

      // Config entry to a file
      node_new->component->type = COMP_FILE;
      node_new->component->data = (char*)file->data_ptr;
      node_new->component->len = file->file_size;
    }
    file = file->next;
  }
  mounted = 1;
  return 0;
}



// fops
int initramfs_write(file *file, const void *buf, size_t len){
  printf("[ERROR][initramfs_write] cannot modify initramfs\r\n");
  return 0;
}
int initramfs_read(file *file, void *buf, size_t len){
  return tmpfs_read(file, buf, len);
}
int initramfs_open(vnode* file_node, file** target){
  return tmpfs_open(file_node, target);
}
int initramfs_close(file *file){
  return tmpfs_close(file);
}

// vops
int initramfs_mkdir(vnode *dir_node, vnode **target, const char *component_name){
  if(mounted){
    printf("[ERROR][initramfs_mkdir] cannot modify initramfs\r\n");
    return 1;
  }
  else
    return tmpfs_mkdir(dir_node, target, component_name);
}
int initramfs_create(vnode *dir_node, vnode **target, const char *component_name){
  if(mounted){
    printf("[ERROR][initramfs_create] cannot modify initramfs\r\n");
    return 1;
  }
  else
    return tmpfs_create(dir_node, target, component_name);
}
int initramfs_lookup(vnode *dir_node, vnode **target, const char *component_name){
  return tmpfs_lookup(dir_node, target, component_name);
}
