#include "main.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <stdbool.h>

// #define HEADER_SIZE ((sizeof(struct header) + 7) & ~7)

#define HEADER_SIZE (sizeof(struct header))

// static header *lists[8];

// lists[0] = NULL;
// lists[1] = NULL;
// lists[2] = NULL;
// lists[3] = NULL;
// lists[4] = NULL;
// lists[5] = NULL;
// lists[6] = NULL;
// lists[7] = NULL;
//
static struct header *lists[8] = {NULL};

void *custom_dealloc(void *memory) {
  if (memory == NULL) {
    return NULL;
  }

  header *block = (header *)((char *)memory - HEADER_SIZE);
  if (block->allocated == 0)
    return NULL;
  block->allocated = 0;
  // block->next = lists[translate(block->size)];
  // lists[translate(block->size)] = block;
  while (100) {
    header *next = (header *)((char *)block + HEADER_SIZE + block->size);
    if (next->allocated != 0) {
      break;
    }
    if (next->allocated == 0) {
      // if (next == NULL) {
      //   break;
      // }

      int nextClass = translate(next->size);
      if (nextClass < 0 || nextClass > 7) {
        // If the size class is out of bounds, 'next' is pointing to random
        // memory at the end of the heap!
        break;
      }

      header *track = lists[translate(next->size)];
      header *previous = NULL;
      int status = 0;
      while (track != NULL) {
        if (track == next) {
          if (previous == NULL) {
            lists[translate(next->size)] = track->next;
          } else {
            previous->next = track->next;
          }
          status = 1;
          break;
        }
        previous = track;
        track = track->next;
      }
      if (status) {
        block->size += HEADER_SIZE + next->size;
        continue;
      }
    }
    break;
  }
  block->next = lists[translate(block->size)];
  lists[translate(block->size)] = block;

  // printf("FREE: ptr=%p size=%zu class=%d\n", block, block->size,
  //        translate(block->size));

  return NULL;
}

void *custom_malloc(int amount, int mode) {
  if (amount <= 0) {
    return NULL;
  }
  amount = (amount + 7) & ~7;

  int listNum = translate(amount);

  header *next = lists[listNum];
  header *prev = NULL;
  header *storageBlock = NULL;

  if (mode == 1) {
    // firstfit
    int found = 0;
    for (int i = listNum; i < 8; i++) {
      prev = NULL;
      next = lists[i];
      while (next != NULL) {
        if (next->size >= amount) {
          storageBlock = next;
          listNum = i;
          found = 1;
          break;
        }
        prev = next;
        next = next->next;
      }
      if (found == 1)
        break;
    }
  }
  if (mode == 3) {
    // bestfit
    uint64_t bestSize = UINT64_MAX;
    header *bestBlock = NULL;
    header *bestPrev = NULL;
    int bestList = -1;

    for (int i = listNum; i < 8; i++) {
      header *curr = lists[i];
      header *currPrev = NULL;

      while (curr != NULL) {
        if (curr->size >= amount && curr->size < bestSize) {
          bestSize = curr->size;
          bestBlock = curr;
          bestPrev = currPrev;
          bestList = i;
        }

        currPrev = curr;
        curr = curr->next;
      }
    }

    storageBlock = bestBlock;
    prev = bestPrev;
    listNum = bestList;
  }

  if (mode == 2) {
    // worstfit
    uint64_t worstSize = 0;
    header *worstBlock = NULL;
    header *worstPrev = NULL;

    // for (int i = listNum; i < 8; i++) {
    header *curr = lists[translate(amount)];
    header *currPrev = NULL;

    while (curr != NULL) {
      if (curr->size >= amount && curr->size > worstSize) {
        worstSize = curr->size;
        worstBlock = curr;
        worstPrev = currPrev;
      }
      currPrev = curr;
      curr = curr->next;
    }
    //}
    storageBlock = worstBlock;
    prev = worstPrev;
  }

  header *newBlock;
  if (storageBlock == NULL) {
    if (amount + HEADER_SIZE > 1024) {
      size_t nSize = (amount + HEADER_SIZE + 16383) & ~(16383);
      newBlock = mmap(NULL, nSize, PROT_READ | PROT_WRITE,
                      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      if (newBlock == (void *)-1)
        return NULL;
      newBlock->size = amount;
    } else {
      newBlock = mmap(NULL, 1024, PROT_READ | PROT_WRITE,
                      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      if (newBlock == (void *)-1)
        return NULL;
      newBlock->size = 1024 - HEADER_SIZE;
    }

    header *newHeader;
    // remainder = newBlock->size - amount - sizeof(header);

    // if (remainder >= sizeof(header) + 8) {
    if (newBlock->size >= amount + HEADER_SIZE + 8) {
      uint64_t remainder = newBlock->size - amount - HEADER_SIZE;
      newHeader = (header *)((char *)newBlock + HEADER_SIZE + amount);
      newHeader->size = remainder;
      newHeader->next = lists[translate(remainder)];
      lists[translate(remainder)] = newHeader;
      newHeader->allocated = 0;
    }
    newBlock->size = amount;
    newBlock->next = NULL;
  }

  if (storageBlock != NULL) {
    if (prev == NULL) {
      lists[listNum] = storageBlock->next;
    } else {
      prev->next = storageBlock->next;
    }
    storageBlock->next = NULL;

    header *newHeader;
    // remainder = storageBlock->size - amount - sizeof(header);

    // if (remainder >= sizeof(header) + 8) {
    if (storageBlock->size >= amount + HEADER_SIZE + 8) {
      uint64_t remainder = storageBlock->size - amount - HEADER_SIZE;

      newHeader = (header *)((char *)storageBlock + HEADER_SIZE + amount);
      newHeader->size = remainder;
      newHeader->next = lists[translate(remainder)];
      lists[translate(remainder)] = newHeader;
      newHeader->allocated = 0;
      storageBlock->size = amount;
      storageBlock->next = NULL;
    }
    storageBlock->allocated = 1;
    // printf("MALLOC: ptr=%p size=%d\n", storageBlock, amount);
    return (void *)(storageBlock + 1);
  }
  newBlock->allocated = 1;

  // printf("MALLOC: ptr=%p size=%d\n", storageBlock, amount);

  return (void *)(newBlock + 1);
}

int main() { return 0; }
