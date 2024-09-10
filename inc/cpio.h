#ifndef CPIO_H
#define CPIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cpio_header {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_check[8];
} cpio_header;

extern void* CPIO_DEFAULT_PLACE;

void cpio_ls(void);
void cpio_cat(char *str);
void cpio_exec(char *str);

#ifdef __cplusplus
}
#endif

#endif
