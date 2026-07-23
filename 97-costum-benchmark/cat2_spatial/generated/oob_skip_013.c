/*
 * TEST: oob_skip_013
 * CATEGORY: gen_oob_skip
 * PURPOSE: OOB from 256-byte obj at offset 1 → adjacent 256B object
 *
 * RSan:   MISS (spatial passes)
 * MixSan: DETECT (63/64)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL  // LAM U57: keep low 57 bits

int main(void) {
    char *a = (char*)malloc(256);
    char *b = (char*)malloc(256);
    
    if (!a || !b) return 1;
    memset(a, 0xAA, 256);
    memset(b, 0xBB, 256 > 256 ? 256 : 256);

    // Strip MemTag before computing distance — LAM encoding puts tags in bits [62:57],
    // which breaks ptrdiff_t subtraction. Use raw addresses for distance, but keep
    // the original tagged pointer 'a' for the access so MemTag_a ≠ MemTag_b triggers.
    uintptr_t raw_a = (uintptr_t)a & MEMTAG_MASK;
    uintptr_t raw_b = (uintptr_t)b & MEMTAG_MASK;
    intptr_t raw_dist = (intptr_t)(raw_b - raw_a);
    if (raw_dist > 0 && raw_dist < 100000) {
        volatile char c = a[raw_dist + 1];
        printf("a[%ld]=%02x\n", (long)(raw_dist + 1), (unsigned char)c);
    } else {
        printf("objects not adjacent, skip\n");
    }
    _exit(0);
}
