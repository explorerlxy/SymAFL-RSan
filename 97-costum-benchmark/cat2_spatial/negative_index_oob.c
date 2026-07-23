/*
 * TEST: negative_index_oob
 * CATEGORY: cat2_spatial
 * PURPOSE: 负索引越界 — 访问 array[-1]，underrun 到前一个对象
 *
 * RSan:   underflow 后 align_down 落到前一个对象上，读取其 bound。
 *         若访问地址落在前一个对象的合法区间 → spatial check 通过 → MISS
 * MixSan: MemTag(ptr) ≠ MemTag(prev_obj's meta) → DETECT (63/64)
 *
 * RATIONALE:
 *   RSan 的 underflow 检测依赖 bound 比较。若负偏移量使其落入前一个对象的
 *   合法区域（而非 redzone），spatial check 可能通过。
 *   MixSan 的 MemTag 提供了额外的保护维度。
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL

int main(void) {
    char *prev = (char*)malloc(32);
    char *curr = (char*)malloc(32);
    if (!prev || !curr) return 1;

    memset(prev, 0xAA, 32);
    memset(curr, 0xBB, 32);

    // Strip MemTag before computing negative offset (LAM U57)
    uintptr_t raw_prev = (uintptr_t)prev & MEMTAG_MASK;
    uintptr_t raw_curr = (uintptr_t)curr & MEMTAG_MASK;
    intptr_t back = (intptr_t)(raw_prev - raw_curr);
    if (back >= -256 && back < 0) {
        volatile char c = curr[back];
        printf("curr[%ld] = %02x\n", (long)back, (unsigned char)c);
    } else {
        printf("objects not adjacent, skip\n");
    }

    _exit(0);
}
