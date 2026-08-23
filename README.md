# manual-memory-allocator
Custom low level functions in C that manually operate on the heap to emulate malloc and free with segregated free lists.

#Features
Custom malloc and dealloc
Uses 3 searching algorithms; first fit, best fit, and worst fit
Uses segregated free lists to hold free blocks
Includes block splitting
Coalesces forward for adjacent blocks, and only forward
Uses mmap to request memory
Checks for invalid input
Includes testing with CTest

#How to use it
void *custom_malloc(int size, int mode);
call the above function with a size >= 1 and a mode (1 = first fit, 2 = best fit, 3 = worst fit) and 
the function will return a block of memory with your specifications.
Example:
void *ptr = custom_malloc(128, 1);  
The above returns a 128 byte block of memory found with the first fit searching algorithm.

void *custom_dealloc(void *);
Call the above function on a pointer to a memory block and it will add the block
back to the appropriate segregated free list.




void *ptr = custom_malloc(128, 1);  // Allocate 128 bytes using First Fit
