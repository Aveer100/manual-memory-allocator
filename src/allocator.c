#include "../include/allocator.h"
#include "../include/allocator_helpers.h"
#include <stdlib.h>
// stdio string and asseert are for tests
#define HEADER_SIZE (sizeof(struct header))

static struct header *lists[8] = {NULL};

// Implementation of free()
void *custom_dealloc(void *memory) {
  if (memory == NULL) {
    return NULL;
  }
  header *block = ((header *)memory) - 1;
  // Prevent double free then mark current block as free
  if (block->allocated == 0) {
    return NULL;
  }
  block->allocated = 0;
  // Coalesce going forward only
  forwardCoalesce(&block, lists);
  // Insert back into the appropriate list
  int listNum = translate(block->size);
  block->next = lists[listNum];
  lists[listNum] = block;
  return NULL;
}

// Implementation of malloc
void *custom_malloc(int amount, int mode) {
  // Validate request
  if (amount <= 0) {
    return NULL;
  }
  if (mode != 1 && mode != 2 && mode != 3) {
    return NULL;
  }
  //  Align to 8 bytes and initialize pointers
  amount = (amount + 7) & ~7;
  int listNum = translate(amount);
  header *prev = NULL;
  header *storageBlock = NULL;
  // First fit
  if (mode == 1) {
    storageBlock = firstFit(amount, lists, &prev, &listNum);
  }
  // Worst fit
  if (mode == 2) {
    // worstfit
    storageBlock = worstFit(amount, lists, &prev, &listNum);
  }
  // Best fit
  if (mode == 3) {
    storageBlock = bestFit(amount, lists, &prev, &listNum);
  }
  header *newBlock;
  // If no block is found
  if (storageBlock == NULL) {
    // If more than the increment is requested
    if (amount + HEADER_SIZE > 1024) {
      newBlock = bigRequest(amount);
    }
    // If less than the increment is requested
    else {
      newBlock = smallRequest();
    }
    // Split, mark, and return the block if less than the increment is needed
    if (newBlock->size >= (int)(amount + (2 * HEADER_SIZE) + 8)) {
      newBlock = splitBlock(newBlock, amount, lists);
    }
    newBlock->allocated = 1;
    return (void *)(newBlock + 1);
  }
  // If an appropriate block is found
  if (storageBlock != NULL) {
    // If it is the first block in the list
    if (prev == NULL) {
      // Remove the block from the list in both cases
      lists[listNum] = storageBlock->next;
    } else {
      prev->next = storageBlock->next;
    }
    storageBlock->next = NULL;
    // Split the block
    if (storageBlock->size >= (int)(amount + (2 * HEADER_SIZE) + 8)) {
      storageBlock = splitBlock(storageBlock, amount, lists);
    }
    storageBlock->allocated = 1;
    // Return the new block
    return (void *)(storageBlock + 1);
  }
  return NULL;
}
