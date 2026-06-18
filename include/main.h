#pragma once

#include <stdint.h>
#include <stdio.h>

typedef struct header {
  uint64_t size;
  struct header *next;
  int allocated;
} header;

struct allocinfo {
  uint64_t free_size;
  uint64_t free_chunks;
  uint64_t largest_free_chunk_size;
  uint64_t smallest_free_chunk_size;
};

void *custom_malloc(int, int);

void *custom_dealloc(void *);
