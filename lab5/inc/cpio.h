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

void cpio_ls();
void cpio_cat(char *str);
void cpio_exec(char *str);
void *load_program(char *name);

extern char *CPIO_DEFAULT_PLACE;
extern char *CPIO_DEFAULT_PLACE_END;
