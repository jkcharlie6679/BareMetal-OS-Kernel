#include "cpio.h"
#include "printf.h"
#include "string.h"

void *CPIO_DEFAULT_PLACE = NULL;

int read_header (char *str, int size) {
  char a[size + 1];
  a[1] = 0;
  for(int i=0; i<size; i++){
    append_str(a, *(str+i));
  }
  return myHex2Int(a);
}

void cpio_ls () {
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  while (strcmp(header + 110, "TRAILER!!!")) {
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    if ((namesize+110) % 4) {
      namesize += (4 - (namesize+110)%4);
    }
    if (filesize % 4) {
      filesize += (4 - filesize%4);
    }
    printf("%s\n\r", header+110);
    header = header + namesize + filesize + 110;
  }
}

void cpio_cat (char *str) {
  cpio_header *cpio = (cpio_header *)CPIO_DEFAULT_PLACE;
  char *header = cpio->c_magic;
  while (strcmp(header+110, "TRAILER!!!")) {
    int namesize = read_header(header + 94, 8);
    int filesize = read_header(header + 54, 8);
    namesize = namesize + namesize%2;
    if ((namesize+110)%4) {
      namesize += (4 - (namesize+110)%4);
    }
    if (filesize % 4) {
      filesize += (4 - filesize%4);
    }
    if (!strcmp(header + 110, str) && filesize != 0) {
      printf("%s\n\r", header + 110 + namesize);
      return;
    }
    header = header + namesize + filesize + 110;
  }
  printf("can't find this file named \"%s\"\n\r", str);
}
