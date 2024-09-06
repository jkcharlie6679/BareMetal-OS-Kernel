#include "stdint.h"
#include "vfs.h"

#ifndef __TMPFS_H_
#define __TMPFS_H_

#define TMPFS_MAX_FILE_SIZE 4096

int tmpfs_setup_mount(filesystem *fs, mount *mount);

// file operations
int tmpfs_write(file *file, const void *buf, size_t len);
int tmpfs_read(file *file, void *buf, size_t len);
int tmpfs_open(vnode *file_node, file **target);
int tmpfs_close(file *file);

// vnode operations
int tmpfs_mkdir(vnode *dir_node, vnode **target, const char *component_name);
int tmpfs_create(vnode *dir_node, vnode **target, const char* component_name);
int tmpfs_lookup(vnode *dir_node, vnode **target, const char* component_name);

#endif