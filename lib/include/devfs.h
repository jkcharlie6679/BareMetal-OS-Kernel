#include "vfs.h"

int devfs_setup_mount(struct filesystem *fs, mount *mount);

// fops
int devfs_write(file *file, const void *buf, size_t len);
int devfs_read(file *file, void *buf, size_t len);
int devfs_open(vnode* file_node, file** target);
int devfs_close(file *file);
// vops

int devfs_mkdir(vnode *dir_node, vnode **target, const char *component_name);
int devfs_create(vnode *dir_node, vnode **target, const char *component_name);
int devfs_lookup(vnode *dir_node, vnode **target, const char *component_name);
