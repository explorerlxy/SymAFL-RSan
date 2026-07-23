/*
 * TEST: large_stride_oob
 * CATEGORY: cat2_spatial
 * PURPOSE: 大跨度越界 — 从一个小对象以大偏移量访问远处的不相关对象
 *
 * RSan:   MISS — 远处对象的 bound 更大，spatial check 通过
 * MixSan: DETECT (63/64) — MemTag mismatch
 *
 * RATIONALE:
 *   模拟"wild pointer"场景 — 指针被错误地增大了很大的偏移量，
 *   落到远处对象的合法区间。RSan 的 spatial check 只看 bound 的大小关系，
 *   不关心两个对象之间的语义隔离。MemTag 提供了这种隔离。
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL

int main(void) {
    char *objs[20];
    for (int i = 0; i < 20; i++) {
        objs[i] = (char*)malloc(64);
        if (!objs[i]) return 1;
        memset(objs[i], (char)i, 64);
    }

    // Strip MemTag before computing distance (LAM U57)
    uintptr_t raw_first = (uintptr_t)objs[0] & MEMTAG_MASK;
    uintptr_t raw_last  = (uintptr_t)objs[19] & MEMTAG_MASK;
    intptr_t far = (intptr_t)(raw_last - raw_first);
    if (far > 0 && far < 100000) {
        volatile char c = objs[0][far];
        printf("objs[0][%ld] = %02x (expected from last obj: %02x)\n",
               (long)far, (unsigned char)c, (unsigned char)objs[19][0]);
    } else {
        printf("objects not adjacent, skip\n");
    }

    _exit(0);
}
