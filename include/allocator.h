#pragma once

#include <stdint.h>

#define ALLOCATION_THRESHOLD 1024

typedef struct header {
  int size;
  struct header *next;
  int allocated;
} header;

void *custom_malloc(int, int);

void *custom_dealloc(void *);
