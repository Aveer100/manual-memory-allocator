#include "../include/allocator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_taboo_inputs(void) {
  printf("Test 1: Taboo Inputs\n");
  assert(custom_malloc(0, 1) == NULL);
  assert(custom_malloc(-1, 1) == NULL);
  assert(custom_dealloc(NULL) == NULL);
  printf("Test Passed\n\n");
}

void test_alloc_and_read_write(void) {
  printf("Test 2: Allocation with Read and Write\n");
  char *memory = (char *)custom_malloc(200, 1);
  assert(memory != NULL);
  // Write data into the allocated memory
  strcpy(memory, "Custom Allocator");
  // Verify the data was written correctly
  assert(strcmp(memory, "Custom Allocator") == 0);
  custom_dealloc(memory);
  printf("Test Passed\n\n");
}

void test_block_reuse(void) {
  printf("Test 3: Block Reuse\n");
  void *firstBlock = custom_malloc(500, 1);
  assert(firstBlock != NULL);
  custom_dealloc(firstBlock);
  // Allocate another block of the same size
  void *secondBlock = custom_malloc(500, 1);
  // Verify the freed block is reused
  assert(secondBlock == firstBlock);
  custom_dealloc(secondBlock);
  printf("Test Passed\n\n");
}

void test_all_fit_algorithms(void) {
  printf("Test 4: First Fit, Best Fit, and Worst Fit Tests\n");
  // Allocate blocks of different sizes
  void *smallBlock = custom_malloc(100, 1);
  void *mediumBlock = custom_malloc(300, 1);
  void *largeBlock = custom_malloc(600, 1);
  assert(smallBlock != NULL);
  assert(mediumBlock != NULL);
  assert(largeBlock != NULL);
  // Free the blocks to populate the free lists
  custom_dealloc(smallBlock);
  custom_dealloc(mediumBlock);
  custom_dealloc(largeBlock);
  // Allocate using the best fit algorithm
  void *bestBlock = custom_malloc(200, 3);
  assert(bestBlock == mediumBlock);
  // Allocate using the worst fit algorithm
  void *worstBlock = custom_malloc(50, 2);
  assert(worstBlock == largeBlock);
  custom_dealloc(bestBlock);
  custom_dealloc(worstBlock);
  printf("Test Passed\n\n");
}

void test_forward_coalescing(void) {
  printf("Test 5: Forward Coalescing Test\n");
  void *firstBlock = custom_malloc(64, 1);
  void *secondBlock = custom_malloc(64, 1);
  void *thirdBlock = custom_malloc(64, 1);
  assert(firstBlock != NULL);
  assert(secondBlock != NULL);
  assert(thirdBlock != NULL);
  // Free the last block first
  custom_dealloc(thirdBlock);
  // Free the middle block to trigger forward coalescing
  custom_dealloc(secondBlock);
  // Allocate a block that fits inside the merged block
  void *mergedBlock = custom_malloc(120, 1);
  // Verify the merged block is reused
  assert(mergedBlock == secondBlock);
  custom_dealloc(firstBlock);
  custom_dealloc(mergedBlock);
  printf("Test Passed\n\n");
}

void test_mmap(void) {
  printf("Test 6: Small and Big Allocation Test\n");
  // Allocate memory smaller than the threshold
  void *smallBlock = custom_malloc(1024, 1);
  assert(smallBlock != NULL);
  // Allocate memory larger than the threshold
  void *largeBlock = custom_malloc(4096, 1);
  assert(largeBlock != NULL);
  // Write to the mapped memory
  memset(largeBlock, 0xAB, 4096);
  custom_dealloc(smallBlock);
  custom_dealloc(largeBlock);
  printf("Test Passed\n\n");
}

void test_double_free(void) {
  printf("Test 7: Double Free Guard\n");
  void *memory = custom_malloc(128, 1);
  assert(memory != NULL);
  // Free the block
  assert(custom_dealloc(memory) == NULL);
  // Verify a second free is handled safely
  assert(custom_dealloc(memory) == NULL);
  printf("Test Passed\n\n");
}

int main(void) {
  test_taboo_inputs();
  test_alloc_and_read_write();
  test_block_reuse();
  test_all_fit_algorithms();
  test_forward_coalescing();
  test_mmap();
  test_double_free();
  return 0;
}
