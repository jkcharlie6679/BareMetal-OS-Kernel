#include "stdint.h"

#define FDT_BEGIN_NODE          (uint64_t)0x00000001
#define FDT_END_NODE            (uint64_t)0x00000002
#define FDT_PROP                (uint64_t)0x00000003
#define FDT_NOP                 (uint64_t)0x00000004
#define FDT_END                 (uint64_t)0x00000009

// big-endian
typedef struct fdt_header {
  uint32_t magic;
  uint32_t totalsize;
  uint32_t off_dt_struct;
  uint32_t off_dt_strings;
  uint32_t off_mem_rsvmap;
  uint32_t version;
  uint32_t last_comp_version;
  uint32_t boot_cpuid_phys;
  uint32_t size_dt_strings;
  uint32_t size_dt_struct;
} fdt_header;

typedef struct fdt_prod{
  uint32_t len;
  uint32_t nameoff;
} fdt_prod;

extern char* dtb_place;
extern uint32_t dtb_size;
typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value, uint32_t name_size);
void fdt_traverse(dtb_callback callback);
void initramfs_callback(uint32_t node_type, char *name, void *value, uint32_t name_size);
