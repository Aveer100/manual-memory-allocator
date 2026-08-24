# manual-memory-allocator
Custom low level functions in C that manually emulate malloc and free with segregated free lists.

## Features
- Custom malloc and dealloc
- Uses 3 searching algorithms; First Fit, Worst Fit, and Best Fit
- Uses segregated free lists to hold free blocks
- Includes block splitting
- Coalesces forward for adjacent blocks
- Uses mmap to request memory
- Checks for invalid input
- Includes testing with CTest

## How to use
``` void *custom_malloc(int size, int mode); ```
- Call the above function with a size >= 1 and a mode (1 = First Fit, 2 = Worst Fit, 3 = Best Fit) and the function will return a block of memory with your specifications.
- Example:
``` void *ptr = custom_malloc(128, 1);  ```
- The above returns a 128 byte block of memory found with the First Fit searching algorithm.
  
``` void *custom_dealloc(void *); ```
- Call the above function on a pointer to a memory block and it will add the block back to the appropriate segregated free list.

## Design
- Free blocks are stored in 8 segregated free lists ranging from sizes
- 1 - 32
- 33 - 64
- 65 - 128
- 129 - 256
- 257 - 512
- 513 - 1024
- 1025 - 2048
- 2049+

Uses the following structure for headers to represent each block
```c
typedef struct header {
  int size;
  struct header *next;
  int allocated;
 } header;
```

Once ``` custom_malloc() ``` is called the function checks for a valid size and mode before any work, the searching algorithm 
starts at the appropriate free list (calculated by a size translation function). If the mode is 1 it will search for the first free block big enough and no further once it's found, if the mode is 2 it will search every block onwards to find the largest free block in the list. If the mode is 3 it will search every block in the lists onwards and choose the smallest free block that is large enough to hold the size. If necessary, the block is split with the user getting the amount they requested and the remainder back into the calculated free list for its size. If no block is found it will call ```mmap()``` with either a small request (requested size + metadata less than 1024 bytes) where it maps a 1024 byte region (part of which is occupied by the header and sentinel) and is split accordingly, or a big request (greater than 1024 bytes) where ```mmap()``` maps memory of the size the user wants + its metadata, rounded up to the nearest full page.

Once ``` custom_dealloc(void* memory) ``` is called the function first checks whether or not the block has already been freed. It gets the pointer for the memory block, marks it as free, and coalesces going strictly forward with the help of sentinels. It then inserts it back into the calculated free list.

## Constraints
The design only coalesces forwards, this is to maintain simplicity and avoid overcomplicating the code as coalescing backwards would require more metadata like a footer structure or a pointer to the previous block. Forward coalescing can be done with strictly the block header structure. The program also does not use ```munmap()```, rather the allocator retains the memory and just stores it in free lists for further use reducing the number of calls to mmap. This also supports simplicity as using ```munmap()``` would require keeping track of mmap boundaries which is particularly difficult in this implementation.

## Requirements
- CMake 3.14 or newer
- C compiler
- Unix-like environment with `mmap` support

## How to run
- Clone the repository then run commands:
```bash
cmake -S . -B build
cmake --build build
```

## Test suite
- Tests invalid allocation input
- Tests double free
- Tests Allocation and deallocation with read write
- Tests block reuse
- Tests all 3 searching algorithms
- Tests forward block coalescing
- Tests ```mmap()```

Run CTest from the build directory:
```bash
ctest
```





