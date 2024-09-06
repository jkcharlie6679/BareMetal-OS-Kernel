#include "devfs.h"
#include "uart.h"
#include "string.h"
#include "tmpfs.h"
#include "printf.h"
#include "malloc.h"

#define DEVFS_UART_NAME "uart"


int devfs_setup_mount(struct filesystem *fs, mount *mount){
  if(mount == NULL){
    printf("[ERROR][devfs_setup_mount] NULL pointer.");
  }
  mount->root = malloc_(sizeof(vnode));
  mount->root->mount = NULL;

  mount->root->component = malloc_(sizeof(vnode_component));
  mount->root->component->name = "";
  mount->root->component->len = 0;
  mount->root->component->entries = NULL;
  mount->root->component->type = COMP_DIR;
  
  mount->root->f_ops = malloc_(sizeof(file_operations));
  mount->root->f_ops->write = devfs_write;
  mount->root->f_ops->read = devfs_read;
  mount->root->f_ops->open = devfs_open;
  mount->root->f_ops->close = devfs_close;

  mount->root->v_ops = malloc_(sizeof(vnode_operations));
  mount->root->v_ops->mkdir = devfs_mkdir;
  mount->root->v_ops->create = devfs_create;
  mount->root->v_ops->lookup = devfs_lookup;

  vnode *dir_node = mount->root;

  // Create uart
  vnode *node_new = NULL;
  int ret = dir_node->v_ops->create(dir_node, &node_new, DEVFS_UART_NAME);
  if(ret == 0){
    node_new->component->type = COMP_FILE;
  }
  return 0;
}


// fops
int devfs_write(file *file, const void *buf, size_t len){
  if(strcmp(file->vnode->component->name, DEVFS_UART_NAME) == 0){
    const char *ptr = buf;
    for(size_t i=0; i<len; i++)
      uart_send(*ptr++);
    return len;
  }
  else{
    printf("Error, devfs_write(), writing to unrecognized device %s\r\n", file->vnode->component->name);
    return 0;
  }
}
int devfs_read(file *file, void *buf, size_t len){
  if(strcmp(file->vnode->component->name, DEVFS_UART_NAME) == 0){
    char *ptr = buf;
    for(size_t i=0; i<len; i++)
      *ptr++ = uart_getc();
    return len;
  }
  else{
    printf("[ERROR][devfs_read] reading from unrecognized device %s\r\n", file->vnode->component->name);
    return 0;
  }
}

int devfs_open(vnode* file_node, file** target){
  return tmpfs_open(file_node, target);
}
int devfs_close(file *file){
  return tmpfs_close(file);
}

// vops
int devfs_mkdir(vnode *dir_node, vnode **target, const char *component_name){
  printf("[ERROR][devfs_mkdir] cannot mkdir with devfs\r\n");
  return 1;
}
int devfs_create(vnode *dir_node, vnode **target, const char *component_name){
  return tmpfs_create(dir_node, target, component_name);
}
int devfs_lookup(vnode *dir_node, vnode **target, const char *component_name){
  return tmpfs_lookup(dir_node, target, component_name);
}
