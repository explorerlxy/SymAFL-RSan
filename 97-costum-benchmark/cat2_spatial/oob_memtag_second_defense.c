/*
 * TEST: oob_memtag_second_defense
 * CATEGORY: cat2_spatial
 * PURPOSE: MemTag为非线性越界提供第二道防线 — MixSan DETECT, RSan MISS
 *
 * RSan:   MISS — OOB访问跳过redzone命中相邻对象合法区间, spatial check通过
 * MixSan: DETECT (概率 63/64) — 两个独立分配的MemTag几乎必然不同,
 *         Stage 2 temporal check充当spatial check的第二道防线
 *
 * RATIONALE:
 *   RSan只有一道防线(spatial check), 一旦OOB跳过redzone就漏检。
 *   MixSan有两道防线: spatial check + temporal check。
 *   第二道防线在63/64概率下补救了第一道防线的盲区。
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL

int main(void) {
    char *objs[5];
    for (int i = 0; i < 5; i++) {
        objs[i] = (char*)malloc(32);
        if (!objs[i]) return 1;
        memset(objs[i], 0xAA + i, 32);
    }

    // Strip MemTag before computing distance (LAM U57)
    uintptr_t raw_a = (uintptr_t)objs[0] & MEMTAG_MASK;
    uintptr_t raw_b = (uintptr_t)objs[1] & MEMTAG_MASK;
    intptr_t dist = (intptr_t)(raw_b - raw_a);
    if (dist < 32 || dist > 8192) {
        printf("objects not adjacent, skip\n");
        _exit(2);
    }

    // OOB access: objs[0]'s tagged pointer jumps into objs[1]'s valid range
    volatile char c = objs[0][dist];
    printf("objs[0][%ld] = %02x (expected objs[1][0] = %02x)\n",
           (long)dist, (unsigned char)c, (unsigned char)objs[1][0]);

    _exit(0);
}
