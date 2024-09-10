#ifndef MALLOC_H
#define MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long size_t;
typedef struct malloc_header {
  unsigned int previous;
  unsigned int chunk;
} malloc_header;

void* simple_malloc(size_t size);

#ifdef __cplusplus
}
#endif

#endif