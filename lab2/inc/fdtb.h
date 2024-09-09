#ifndef FDTB_H
#define FDTB_H

#ifdef __cplusplus
extern "C" {
#endif

// #define uint32_t unsigned int
#include <stdint.h>

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

// big-endian
struct fdt_header {
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
};

struct fdt_prod{
  uint32_t len;
  uint32_t nameoff;
};

extern char* fdtb_place;

typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value, uint32_t name_size);
void fdt_traverse(dtb_callback callback);
void initramfs_callback(uint32_t node_type, char *name, void *value, uint32_t name_size);

#ifdef __cplusplus
}
#endif

#endif