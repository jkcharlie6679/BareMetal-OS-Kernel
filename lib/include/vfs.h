#include "stdint.h"

#ifndef __vfs_H_
#define __vfs_H_

#define TMPFS_MAX_COMPONENT_NAME 32
#define NAME_LEN 16
#define TMPFS_MAX_ENTRY 16
#define VFS_MAX_DEPTH 64
#define O_CREAT 00000100
#define TMPFS_MAX_PATH_LEN 255
#define VFS_PROCESS_MAX_OPEN_FILE 16
#define CURRENT_DIR "."
#define PARENT_DIR ".."

typedef uint64_t size_t;

typedef enum comp_type{
  COMP_FILE = 1,
  COMP_DIR  // directory
} comp_type;

typedef struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  struct vnode_component *component;
} vnode;

typedef struct vnode_component{
  char *name;
  enum comp_type type;
  size_t len;       // COMP_DIR: entry count in this directory; COM_FILE: file size in byte
  union {
    vnode **entries; // for type of COMP_DIR
    char  *data;     // for type of COM_FILE
  };
} vnode_component;

// file handle
typedef struct file {
  struct vnode* vnode;
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
} file;

typedef struct mount {
  struct vnode* root;
  struct filesystem* fs;
} mount;

typedef struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem *fs, mount *mount);
} filesystem;

typedef struct file_operations {
  int (*write)(file *file, const void* buf, size_t len);
  int (*read)(file *file, void* buf, size_t len);
  int (*open)(vnode *file_node, file **target);
  int (*close)(file *file);
  // long lseek64(struct file* file, long offset, int whence);
} file_operations;

typedef struct vnode_operations {
  int (*lookup)(vnode *dir_node, vnode **target, const char *component_name);
  int (*create)(vnode *dir_node, vnode **target, const char *component_name);
  int (*mkdir)(vnode *dir_node, vnode **target, const char *component_name);
} vnode_operations;

int register_filesystem(filesystem* fs);

int vfs_open(const char* pathname, int flags, struct file** target);

int vfs_close(file *file);

int vfs_write(file *file, const void* buf, size_t len);

int vfs_read(file *file, void* buf, size_t len);

int vfs_mkdir(const char* pathname);

int vfs_mount(const char* target, const char* file_name);

int lookup_path(const char* pathname, vnode *node, vnode **target, int create);
int vfs_lookup(const char* pathname, vnode **target);

void vfs_dump_under(vnode *node, int depth);
void vfs_dump_root();

int to_abs_path(char *abs_path, const char *cwd, const char *path);

#endif
