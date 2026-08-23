#include "../include/allocator_helpers.h"
#include "../include/allocator.h"
#include <limits.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>

#define HEADER_SIZE (sizeof(struct header))

// Map a requested size to an apprpriate free list
int translate(int num) {
  if (num <= 32) {
    return 0;
  }
  if (num <= 64) {
    return 1;
  }
  if (num <= 128) {
    return 2;
  }
  if (num <= 256) {
    return 3;
  }
  if (num <= 512) {
    return 4;
  }
  if (num <= 1024) {
    return 5;
  }
  if (num <= 2048) {
    return 6;
  }
  return 7;
}

// Create a sentinel for forward coalescing safety
void addSentinel(header *newBlock, int size) {
  header *mySentinel = (header *)(((char *)newBlock) + HEADER_SIZE + size);
  mySentinel->allocated = 1;
  mySentinel->size = 0;
  mySentinel->next = NULL;
}

// Handles using mmap to request memory greater than 1024 bytes
header *bigRequest(int amount) {
  // Align the amount to the whole number of pages that will be needed
  header *newBlock;
  int pageSize = (int)sysconf(_SC_PAGESIZE);
  int newAmount =
      (amount + (2 * HEADER_SIZE) + (pageSize - 1)) & ~(pageSize - 1);
  newBlock = mmap(NULL, newAmount, PROT_READ | PROT_WRITE,
                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  // Safety check
  if (newBlock == (void *)-1) {
    return NULL;
  }
  newBlock->size = amount;
  addSentinel(newBlock, amount);
  return newBlock;
}

// Handles using mmap to request memory less than 1024 bytes
header *smallRequest() {
  header *newBlock;
  newBlock = mmap(NULL, ALLOCATION_THRESHOLD, PROT_READ | PROT_WRITE,
                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  // Safety check
  if (newBlock == (void *)-1) {
    return NULL;
  }
  newBlock->size = ALLOCATION_THRESHOLD - (2 * HEADER_SIZE);
  addSentinel(newBlock, newBlock->size);
  return newBlock;
}

// Handles spliting a block in two pieces
header *splitBlock(header *newBlock, int amount, header **lists) {
  // Create the second block
  header *newHeader;
  uint64_t remainder = (newBlock->size - (amount + HEADER_SIZE));
  newHeader = (header *)((char *)newBlock + HEADER_SIZE + amount);
  newHeader->size = remainder;
  // Insert the new block into the correct free list
  newHeader->next = lists[translate(remainder)];
  lists[translate(remainder)] = newHeader;
  newHeader->allocated = 0;
  // Update the original block and return it to the user
  newBlock->size = amount;
  newBlock->next = NULL;
  return newBlock;
}

// Implements the first fit block searching algorithm
header *firstFit(int amount, header **lists, header **prev, int *listNum) {
  amount = (amount + 7) & ~7;
  // Start at the appropriate free list
  *listNum = translate(amount);
  header *next = NULL;
  header *storageBlock = NULL;
  int search = 0;
  // Prepare to possibly iterate through multiple free lists
  for (int i = *listNum; i < 8; i++) {
    *prev = NULL;
    next = lists[i];
    // Traverse the current free list
    while (next != NULL) {
      // If the next block in the current list can fit the amount into it, the
      // algorithm is done
      if (next->size >= amount) {
        storageBlock = next;
        *listNum = i;
        search = 1;
        break;
      }
      *prev = next;
      next = next->next;
    }
    if (search == 1) {
      break;
    }
    // Go to the next free list if no block is found
  }
  return storageBlock;
}

// Implements the best fit block searching algorithm
header *bestFit(int amount, header **lists, header **prev, int *listNum) {
  amount = (amount + 7) & ~7;
  // Start at the appropriate free list
  *listNum = translate(amount);
  header *next = NULL;
  header *storageBlock = NULL;
  header *nextPrev = NULL;
  int threshold = INT_MAX;
  // Prepare to iterate through all free lists after and including the initial
  // chosen list
  for (int i = *listNum; i < 8; i++) {
    next = lists[i];
    nextPrev = NULL;
    while (next != NULL) {
      // If the next block in the current list simutaneously fits the amount
      // into it and is smaller than the previous storage block, make it the new
      // storage block
      if (next->size >= amount && next->size < threshold) {
        threshold = next->size;
        storageBlock = next;
        *prev = nextPrev;
        *listNum = i;
      }
      nextPrev = next;
      next = next->next;
    }
    // Go to the next free list
  }
  return storageBlock;
}

// Implements the worst fit block searching algorithm
header *worstFit(int amount, header **lists, header **prev, int *listNum) {
  amount = (amount + 7) & ~7;
  // Start at the appropriate free list
  *listNum = translate(amount);
  header *next = NULL;
  header *storageBlock = NULL;
  header *nextPrev = NULL;
  uint64_t threshold = 0;
  // Prepare to iterate through all free lists after and including the initial
  // chosen list
  for (int i = *listNum; i < 8; i++) {
    next = lists[i];
    nextPrev = NULL;
    while (next != NULL) {
      // If the next block fits the amount and is larger than the previous
      // storage block, make it the new storage block
      if (next->size >= amount && next->size > (int)threshold) {
        threshold = next->size;
        storageBlock = next;
        *prev = nextPrev;
        *listNum = i;
      }
      nextPrev = next;
      next = next->next;
    }
    // Go to the next free list
  }
  return storageBlock;
}

// Function to coalesce free blocks going forwards only
void forwardCoalesce(header **block, header **lists) {
  while (true) {
    header *next = (header *)((char *)*block + HEADER_SIZE + (*block)->size);
    // If the end of the mmap region is reached, break
    if (next->size == 0 && next->allocated == 1) {
      break;
    }
    // Check that the next block is a free block
    if (next->allocated) {
      break;
    }
    int listNum = translate(next->size);
    //  Remove the next block from its free list
    header *temp = lists[listNum];
    header *prev = NULL;
    int found = 0;
    while (temp != NULL) {
      if (temp == next) {
        if (prev == NULL) {
          lists[listNum] = temp->next;
        } else {
          prev->next = temp->next;
        }
        found = 1;
        break;
      }
      prev = temp;
      temp = temp->next;
    }
    if (found == 0) {
      break;
    }
    // Merge it with next block
    (*block)->size += HEADER_SIZE + next->size;
  }
}
