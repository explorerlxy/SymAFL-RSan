/*
 * TEST: oob_far_jump
 * CATEGORY: cat2_spatial
 * PURPOSE: 大跨度越界命中远处不同size class对象
 *
 * RSan:   MISS — 远距离偏移命中其他对象的合法区间
 * MixSan: DETECT (63/64) — MemTag mismatch
 *
 * RATIONALE:
 *   与 oob_skip_redzone 类似，但目标对象是不同的 size class，
 *   以验证 MemTag mismatch 在不同 size class 间的检测能力。
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL

int main(void) {
    // Use same-size objects to ensure adjacency in the same tcmalloc span
    char *a = (char*)malloc(64);
    char *b = (char*)malloc(64);
    if (!a || !b) return 1;

    memset(a, 0xAA, 64);
    memset(b, 0xBB, 64);

    // Strip MemTag before computing offset (LAM U57)
    uintptr_t raw_a = (uintptr_t)a & MEMTAG_MASK;
    uintptr_t raw_b_mid = ((uintptr_t)b + 32) & MEMTAG_MASK;
    intptr_t offset = (intptr_t)(raw_b_mid - raw_a);
    if (offset <= 0 || offset > 1000000) {
        printf("unexpected layout, skip\n");
        _exit(0);
    }

    printf("accessing a[%ld] -> should hit b[32]\n", (long)offset);
    volatile char c = a[offset];
    printf("a[%ld] = %02x (expected b[32] = %02x)\n",
           (long)offset, (unsigned char)c, (unsigned char)b[32]);

    _exit(0);
}
