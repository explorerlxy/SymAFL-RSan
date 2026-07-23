/* Ring buffer overflow size=8 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL
int main() {
    int *ring = (int*)malloc(32);
    int *guard = (int*)malloc(32);
    if (!ring || !guard) return 1;
    memset(guard, 0xFF, 32);
    uintptr_t raw_ring = (uintptr_t)ring & MEMTAG_MASK;
    uintptr_t raw_guard = (uintptr_t)guard & MEMTAG_MASK;
    intptr_t dist = (intptr_t)(raw_guard - raw_ring);
    if (dist > 0 && dist < 100000) {
        int *target = (int*)((char*)ring + dist);
        for (int i = 0; i < 4; i++) target[i] = i * 10;
        printf("guard[0]=0x%x\n", guard[0]);
    } else { printf("skip\n"); }
    _exit(0);
}
