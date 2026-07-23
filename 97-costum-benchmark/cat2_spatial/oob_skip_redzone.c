/*
 * TEST: oob_skip_redzone
 * CATEGORY: cat2_spatial
 * PURPOSE: 越界访问跳过redzone命中相邻对象的合法区间
 *
 * RSan:   MISS — a+offset落在b的合法区间内，spatial check通过
 * MixSan: DETECT (63/64) — MemTag(a) ≠ MetaMemTag(b) → Stage 2触发
 *
 * RATIONALE:
 *   RSan 的 spatial check 比较 ptr+offset 与 bound。当越界量足够大（超过 redzone），
 *   访问地址落入下一个已分配对象的合法区间时，spatial check 因 bound(b) 更大而通过 → 漏报。
 *   MixSan 的 MemTag check 检查指针的 MemTag 与目标地址元数据的 MemTag 是否匹配。
 *   由于 a 和 b 是不同的分配（有不同的 MemTag），MemTag mismatch 触发 →
 *   补救了 spatial check 的盲区。
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL

int main(void) {
    char *a = (char*)malloc(32);
    char *b = (char*)malloc(32);
    if (!a || !b) return 1;

    memset(a, 0xAA, 32);
    memset(b, 0xBB, 32);

    // Strip MemTag before computing distance (LAM U57)
    uintptr_t raw_a = (uintptr_t)a & MEMTAG_MASK;
    uintptr_t raw_b = (uintptr_t)b & MEMTAG_MASK;
    intptr_t skip_dist = (intptr_t)(raw_b - raw_a);
    if (skip_dist < 32 || skip_dist > 8192) {
        printf("unexpected layout, skip\n");
        _exit(2);
    }

    // a[skip_dist] → carries MemTag_a, addresses slot_b → MemTag mismatch → DETECT
    volatile char c = a[skip_dist];
    printf("a[%ld] = %02x (expected b[0] = %02x)\n",
           (long)skip_dist, (unsigned char)c, (unsigned char)b[0]);

    _exit(0);
}
