#include <stdint.h>
#include <stdlib.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL
#define MEMTAG_SHIFT 57
#define MEMTAG_BITS 0x3FULL

int main(void) {
  char *ptr = malloc(16);
  if (ptr == NULL)
    return 1;

  ptr[0] = 0x5a;

  uintptr_t tagged = (uintptr_t)ptr;
  uintptr_t memtag = (tagged >> MEMTAG_SHIFT) & MEMTAG_BITS;
  uintptr_t mismatched_tag = (memtag + 1) & MEMTAG_BITS;
  char *mismatched = (char *)((tagged & MEMTAG_MASK) |
                              (mismatched_tag << MEMTAG_SHIFT));

  volatile char value = mismatched[0];
  return value == 0x5a ? 0 : 2;
}
