#include "tmpfs.h"
#include "printf.h"
#include "malloc.h"
#include "string.h"

int tmpfs_setup_mount(struct filesystem *fs, mount *mount){
  if(mount == NULL){
    printf("[ERROR][tmpfs_setup_mount] no pointer");
  }
  // char tmp_name[TMPFS_MAX_COMPONENT_NAME];
  // tmp_name[0] = '\0';
  // if(mount->root != NULL){
  //   strcpy(tmp_name, mount->root->component->name);
  // }
  mount->root = malloc_(sizeof(vnode));
  mount->root->mount = NULL;
  mount->root->component = malloc_(sizeof(vnode_component));
  mount->root->component->type = COMP_DIR;
  mount->root->component->len = 0;
  mount->root->component->name = "";
  // strcpy(mount->root->component->name, tmp_name);
  mount->root->component->entries = NULL;

  mount->root->f_ops = malloc_(sizeof(file_operations));
  mount->root->f_ops->write = tmpfs_write;
  mount->root->f_ops->read = tmpfs_read;
  mount->root->f_ops->open = tmpfs_open;
  mount->root->f_ops->close = tmpfs_close;

  mount->root->v_ops = malloc_(sizeof(vnode_operations));
  mount->root->v_ops->mkdir = tmpfs_mkdir;
  mount->root->v_ops->create = tmpfs_create;
  mount->root->v_ops->lookup = tmpfs_lookup;
  return 0;
}

int tmpfs_write(file *file, const void *buf, size_t len){
  if(file == NULL){
    printf("[ERROR][tmpfs_write] no file\n\r");
    return -1;
  }
  vnode *node = file->vnode;
  if(node == NULL){
    printf("[ERROR][tmpfs_write] node of file is NULL\n\r");
    return -1;
  }
  vnode_component *component = node->component;
  if(component == NULL){
    printf("[ERROR][tmpfs_write] component of file is NULL\n\r");
    return -1;
  }
  if(component->type != COMP_FILE){
    printf("[ERROR][tmpfs_write] type of file is ERROR\n\r");
    return -1;
  }

  const size_t ideal_final_pos = file->f_pos + len;
  if(ideal_final_pos > component->len && component->len < TMPFS_MAX_FILE_SIZE){
    size_t new_len = ideal_final_pos >= TMPFS_MAX_FILE_SIZE ? TMPFS_MAX_FILE_SIZE : ideal_final_pos;
    new_len = new_len > TMPFS_MAX_FILE_SIZE ? TMPFS_MAX_FILE_SIZE : new_len;
    char *new_space = malloc_(sizeof(char) * new_len);
    if(component->len > 0){
      memcpy_(new_space, component->data, component->len);
      free(component->data);
    }
    component->len = new_len;
    component->data = new_space;
  }
  const size_t wrtie_able = ideal_final_pos >= TMPFS_MAX_FILE_SIZE ? (TMPFS_MAX_FILE_SIZE-file->f_pos) : len;
  memcpy_(component->data + file->f_pos, buf, wrtie_able);
  file->f_pos += wrtie_able;
  return wrtie_able;
}

int tmpfs_read(file *file, void *buf, size_t len){
  if(file == NULL){
    printf("[ERROR][tmpfs_read] no file\n\r");
    return -1;
  }
  vnode *node = file->vnode;
  if(node == NULL){
    printf("[ERROR][tmpfs_read] node of file is NULL\n\r");
    return -1;
  }
  vnode_component *component = node->component;
  if(component == NULL){
    printf("[ERROR][tmpfs_read] component of file is NULL\n\r");
    return -1;
  }
  if(component->type != COMP_FILE){
    printf("[ERROR][tmpfs_read] type of file is ERROR\n\r");
    return -1;
  }

  const size_t ideal_final_pos = file->f_pos + len;
  const size_t read_able = ideal_final_pos >= component->len ? (component->len - file->f_pos) : len;
  memcpy_(buf, component->data, read_able);
  file->f_pos += read_able;
  return read_able;
}

int tmpfs_open(vnode* file_node, file** target){
  if(file_node->component->type != COMP_FILE){
    printf("[ERROR][tmpfs_open] node is not file\n\r");
    return -1;
  }
  *target = malloc_(sizeof(file));
  (*target)->f_ops = file_node->f_ops;
  (*target)->f_pos = 0;
  (*target)->vnode = file_node;
  return 0;
}

int tmpfs_close(file *file){
  if(file == NULL){
    printf("[ERROR][tmpfs_close] no file\n\r");
    return -1;
  }
  free(file);
  return 0;
}

int tmpfs_mkdir(vnode *dir_node, vnode **target, const char *component_name){
  if(dir_node->component->type != COMP_DIR){
    printf("[ERROR][tmpfs_mkdir] The vnode is not folder, tmpfs_mkdir() error at '%s'\n\r", component_name);
    return -1;
  }
  int result = tmpfs_create(dir_node, target, component_name);
  if(result == 0){
    (*target)->component->type = COMP_DIR;
    (*target)->component->entries = 0;
    return 0;
  }else if(result == 1){
    return 0;
  }

  return -1;
}

int tmpfs_create(vnode *dir_node, vnode **target, const char* component_name){
  if(dir_node->component->type != COMP_DIR){
    printf("[ERROR][tmpfs_create] tmpfs_create() the vnode isn't folder at '%s'\n\r", component_name);
    return -1;
  }

  // Return if already exist
  if(tmpfs_lookup(dir_node, target, component_name) == 0){
    return 1;
  }

  if(dir_node->component->len >= TMPFS_MAX_ENTRY){
    printf("[ERROR][tmpfs_create] '%s', no more entry can create\n\r", component_name);
    return -1;
  }

  *target = malloc_(sizeof(vnode));
  (*target)->component = malloc_(sizeof(vnode_component));
  (*target)->component->name = malloc_(sizeof(char) * (NAME_LEN));
  strcpy((*target)->component->name, component_name);
  // printf("name: %s\n\r", (*target)->component->name);
  (*target)->component->data = NULL;
  (*target)->component->len = 0;

  (*target)->f_ops = dir_node->f_ops;
  (*target)->v_ops = dir_node->v_ops;
  (*target)->mount = NULL; // no mount point
  if(dir_node->component->entries == NULL) 
    dir_node->component->entries = malloc_(sizeof(vnode*) * TMPFS_MAX_ENTRY);
  dir_node->component->entries[dir_node->component->len++] = *target;
  return 0;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name){
  if(dir_node->component->type != COMP_DIR){
    printf("[ERROR][tmpfs_lookup] tmpfs_lookup() the vnode isn't folder at '%s'\n\r", component_name);
    return -1;
  }
  vnode *entry = NULL;
  for(size_t i=0; i<dir_node->component->len; i++){
    entry = dir_node->component->entries[i];
    if(strcmp(component_name, entry->component->name) == 0){
      *target = entry->mount == NULL ? entry : (entry->mount->root);
      return 0;
    }
  }
  return -1;
}
