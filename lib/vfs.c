#include "vfs.h"
#include "tmpfs.h"
#include "string.h"
#include "printf.h"
#include "malloc.h"
#include "cpio.h"
#include "devfs.h"

mount* rootfs;

static char *pop_first_component(char *pathname);
static int first_component(char *pathname, char *component_name);

int register_filesystem(filesystem *fs) {
  if(strcmp(fs->name, "tmpfs") == 0){
    fs->setup_mount = tmpfs_setup_mount;
    return 0;  
  }
  printf("[ERROR][register_filesystem] fs not supported!\n\r");
  return -1;
}

int vfs_mount(const char* target, const char* file_name){
  if(strcmp(target, "/") == 0){
    rootfs = malloc_(sizeof(mount));
    rootfs->fs = malloc_(sizeof(filesystem));
    rootfs->fs->name = malloc_(NAME_LEN);
    strcpy((char *)rootfs->fs->name, file_name);
    register_filesystem(rootfs->fs);
    return rootfs->fs->setup_mount(rootfs->fs, rootfs);
  }else if(strcmp(target, "/initramfs") == 0){
    vfs_mkdir(target);
    vnode *vnode_mount = NULL;
    int result = vfs_lookup(target, &vnode_mount);
    if(result == 0){
      vnode_mount->mount = malloc_(sizeof(mount));
      vnode_mount->mount->fs = malloc_(sizeof(filesystem));
      vnode_mount->mount->fs->name = malloc_(NAME_LEN);
      // vnode_mount->mount->root = vnode_mount;   // for pass the node name
      strcpy((char *)vnode_mount->mount->fs->name, file_name);
      vnode_mount->mount->fs->setup_mount = initramfs_setup_mount;
      return vnode_mount->mount->fs->setup_mount(vnode_mount->mount->fs, vnode_mount->mount);
    }
  }else if(strcmp(target, "/dev") == 0){
    vfs_mkdir(target);
    vnode *vnode_mount = NULL;
    int result = vfs_lookup(target, &vnode_mount);
    if(result == 0){
      vnode_mount->mount = malloc_(sizeof(mount));
      vnode_mount->mount->fs = malloc_(sizeof(filesystem));
      vnode_mount->mount->fs->name = malloc_(NAME_LEN);
      // vnode_mount->mount->root = vnode_mount;   // for pass the node name
      strcpy((char *)vnode_mount->mount->fs->name, file_name);
      vnode_mount->mount->fs->setup_mount = devfs_setup_mount;
      return vnode_mount->mount->fs->setup_mount(vnode_mount->mount->fs, vnode_mount->mount);
    }
  }else{
    vnode *vnode_mount = NULL;
    int result = vfs_lookup(target, &vnode_mount);
    if(result == 0){
      vnode_mount->mount = malloc_(sizeof(mount));
      vnode_mount->mount->fs = malloc_(sizeof(filesystem));
      vnode_mount->mount->fs->name = malloc_(NAME_LEN);
      vnode_mount->mount->root = vnode_mount;   // for pass the node name
      strcpy((char *)vnode_mount->mount->fs->name, file_name);
      register_filesystem(vnode_mount->mount->fs);
      return vnode_mount->mount->fs->setup_mount(vnode_mount->mount->fs, vnode_mount->mount);
    }else{
      printf("[ERROR][vfs_mount] mount error no dir\n\r");
    }
  }
  printf("[ERROR][vfs_mount] mount error\n\r");
  return -1;
}


int lookup_path(const char* pathname, vnode *dir_node, vnode **target, int create){
  char *rest_path = NULL; 
  char component_name[TMPFS_MAX_COMPONENT_NAME];
  first_component((char *)pathname, component_name);
  rest_path = pop_first_component((char *)pathname);
  if(dir_node->mount != NULL){
    return lookup_path(pathname, dir_node->mount->root, target, create);
  }

  if(dir_node->component->type != COMP_DIR){
    printf("[ERROR][lookup_path]fail at pathname: %s. the type isn't folder\n\r", pathname);
    return -1;
  }

  if(create == 0){
    if(dir_node->v_ops->lookup(dir_node, target, component_name) == 0){
      dir_node = *target;
      if(rest_path[0] != '\0' && dir_node->component->type == COMP_DIR){
        return lookup_path(rest_path, dir_node, target, create);
      }else if(rest_path[0] != '\0' && dir_node->component->type == COMP_FILE){
        printf("[ERROR][lookup_path] find the file '%s' not dir.\n\r", dir_node->component->name);
        return -1;
      }else if(rest_path[0] == '\0'){
        return 0;
      }
    }
    return -1;
  }else if(create == 1){
    char *rest_comps[VFS_MAX_DEPTH];
    char path_copy[TMPFS_MAX_PATH_LEN];
    strcpy(path_copy, pathname);
    int path_count = spilt_strings(rest_comps, path_copy, "/");
    for(int i; i<path_count; i++){
      if(rest_comps[i][0] == '\0')
        continue;
      if(dir_node->v_ops->mkdir(dir_node, target, rest_comps[i]) == 0){
        dir_node = *target;
      }else{
        printf("[ERROR][lookup_path]\n\r");
      }
    }
  }
  return 0;
}

int vfs_lookup(const char* pathname, vnode **target){
  if(strcmp(pathname, "/") == 0){
    *target = rootfs->root;
    return 0;
  }
  return lookup_path(pathname, rootfs->root, target, 0);
}


int vfs_open(const char* pathname, int flags, file **target) {
  vnode *node = NULL;

  int result = vfs_lookup(pathname, &node);

  if(result != 0 && flags & O_CREAT){
    int result = lookup_path(pathname, rootfs->root, &node, 1);
    if(result == 0){
      node->component->type = COMP_FILE;
      node->component->data = NULL;
      // printf("[INFO][vfs_open] create the new file '%s'\n\r", node->component->name);
    }else{
      printf("[ERROR][vfs_open] create new file '%s' error\n\r", pathname);
      return -1;
    }
  }else if(result != 0){
    printf("[ERROR][vfs_open] can't find the file: %s\n\r", pathname);
    return -1;
  }

  result = node->f_ops->open(node, target);
  if(result == 0){
    (*target)->flags = flags;
    // printf("[INFO][vfs_open] open the file '%s'\n\r", node->component->name);
  }else{
    printf("[ERROR][vfs_open] can't open file '%s' at path: %s\n\r", node->component->name, pathname);
  }
  return result;
}

int vfs_close(file* file) {
  // 1. release the file handle
  // 2. Return error code if fails
  return file->f_ops->close(file);
}

int vfs_write(file* file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  return file->f_ops->write(file, buf, len);
}

int vfs_read(file* file, void* buf, size_t len) {
  // 1. read min(len, readable size) byte to buf from the opened file.
  // 2. block if nothing to read for FIFO type
  // 2. return read size or error code if an error occurs.
  return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char* pathname){
  // printf("mkdir %s\n\r", pathname);
  vnode *node = NULL;
  if(vfs_lookup(pathname, &node) == 1){
    printf("[WARNING][vfs_mkdir]'%s' is already existed.\n\r", pathname);
    return 0;
  }else{
    if(lookup_path(pathname, rootfs->root, &node, 1) == 0){
      // printf("[INFO][vfs_mkdir] create the dir '%s'\n\r", pathname);
      return 0;
    }else{
      printf("[ERROR][vfs_mkdir] fail to create '%s'.\n\r", pathname);
    }
  }
  return -1;
}


static char *pop_first_component(char *pathname){ // return the string pointer right after '/' or '\0'
  int i = 0;
  if(pathname[i] == '/') 
    i++;                           // skip leading '/'
  while(pathname[i] != '\0' && pathname[i] != '/') 
    i++; // skip first directory
  if(pathname[i] == '/') 
    i++;                           // skip leading '/'
  return &pathname[i];
}

static int first_component(char *pathname, char *component_name){
  int i = 0;
  if(pathname[i] == '/') 
    pathname++;                    // skip leading '/'
  while(pathname[i] != '\0' && pathname[i] != '/'){     // copy until end of string or '/'
    component_name[i] = pathname[i];
    i++;
  }
  component_name[i] = '\0';
  return 0;
}

void vfs_dump_under(vnode *node, int depth){
  if(node->mount != NULL){
    printf("mount: %s\n\r", node->component->name);
    vfs_dump_under(node->mount->root, depth);
  }
  else if(node->component->type == COMP_DIR){
    vnode *entry = NULL;
    for(size_t i=0; i<node->component->len; i++){
      entry = node->component->entries[i];
      for(int k=0; k<depth; k++) 
        printf("    ");
      printf("%lu, 0x%lX, %s, type=%d, len=%lu\r\n", i, (uint64_t)entry, entry->component->name, entry->component->type, entry->component->len);
      if(entry->component->type == COMP_DIR)
        vfs_dump_under(entry, depth + 1);
    }
  }else{
    printf("Error, vfs_dump_under(), node type is not dir. type=%d, name=%s\r\n", 
      node->component->type, node->component->name);
  }
}

void vfs_dump_root(){
  vfs_dump_under(rootfs->root, 0);
}

int to_abs_path(char *abs_path, const char *cwd, const char *path){
  // Input abs path, copy directly and return
  if(path[0] == '/'){
    strcpy(abs_path, path);
    return 0;
  }
  // Return if cwd not starts or not ends with '/'
  if(cwd[0] != '/' || cwd[strlen((char *)cwd)-1] != '/'){
    printf("Error, to_abs_path(), cwd should stars and ends with \"/\", cwd=%s\r\n", cwd);
    return 1;
  }
  // Concatenate cwd and input path
  char untranslate[TMPFS_MAX_PATH_LEN]; // "." and ".." are untranslated yet
  untranslate[0] = '\0';
  strcat_(untranslate, cwd);
  strcat_(untranslate, path);
  // Spilt string by "/"
  char *comps[VFS_MAX_DEPTH]; // component string array
  int count = spilt_strings(comps, untranslate, "/");
  // Translate "." and ".."
  int w = 0;  // write(update) index, w-- is like pop, w++ is like push. in place FILO
  int r = 0;  // read index, always advance by 1. in place FIFO
  while(w < count && r < count){
    // Skip blank and "."
    while(comps[r][0] == '\0' || strcmp(comps[r], CURRENT_DIR) == 0)
      r++;
    // Handle ".."
    if(strcmp(comps[r], PARENT_DIR) == 0){
      // when stack empty (w==0) or stack top is "..", then push ".."
      if(w == 0 || strcmp(comps[w-1], PARENT_DIR)==0)
        comps[w++] = comps[r];   // push ".."
      else
        comps[w--] = NULL;    // pop, just "w--;" also does the job
    }
    // Push normal component
    else{
      comps[w++] = comps[r];
    }
    r++;
  }
  count = w; // items after w are ignored
  abs_path[0] = '\0'; // clear string
  for(int i=0; i<count; i++){
    strcat_(abs_path, "/");
    strcat_(abs_path, comps[i]);
  }
  return 0;
}

