#include "vfs.h"
#include "tmpfs.h"

#define USER_PROGRAM_SPACE 0x900000
#define USER_PROGRAM_MAX_SIZE 0x500000

typedef struct cpio_header {
  char	   c_magic[6];
  char	   c_ino[8];
  char	   c_mode[8];
  char	   c_uid[8];
  char	   c_gid[8];
  char	   c_nlink[8];
  char	   c_mtime[8];
  char	   c_filesize[8];
  char	   c_devmajor[8];
  char	   c_devminor[8];
  char	   c_rdevmajor[8];
  char	   c_rdevminor[8];
  char	   c_namesize[8];
  char	   c_check[8];
} cpio_header;


typedef struct cpio_file{
  uint8_t *pathname;       // string
  uint64_t file_size;
  uint8_t *data_ptr;
  struct cpio_file* next;
} cpio_file_ll; // ll for linked list

void cpio_ls();
void cpio_cat(char *str);
void cpio_exec(char *str);
void *load_program(char *name);
int initramfs_setup_mount(struct filesystem *fs, mount *mount);

// fops
int initramfs_write(file *file, const void *buf, size_t len);
int initramfs_read(file *file, void *buf, size_t len);
int initramfs_open(vnode* file_node, file** target);
int initramfs_close(file *file);

// vops
int initramfs_mkdir(vnode *dir_node, vnode **target, const char *component_name);
int initramfs_create(vnode *dir_node, vnode **target, const char *component_name);
int initramfs_lookup(vnode *dir_node, vnode **target, const char *component_name);

extern char *CPIO_DEFAULT_PLACE;
extern char *CPIO_DEFAULT_PLACE_END;
