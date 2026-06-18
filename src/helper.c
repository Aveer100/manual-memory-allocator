// #include <math.h>
#include <stdio.h>

// int translate(int num) { return (int)(ceil(log2(num)) - 5); }
int translate(size_t size) {
  if (size <= 32)
    return 0;
  if (size <= 64)
    return 1;
  if (size <= 128)
    return 2;
  if (size <= 256)
    return 3;
  if (size <= 512)
    return 4;
  if (size <= 1024)
    return 5;
  if (size <= 2048)
    return 6;
  return 7;
}
