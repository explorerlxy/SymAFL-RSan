/* SB bypass: OOB pointer as uintptr_t stored in heap uintptr_t array -> reload -> deref.
 * SB shadow stack metadata is bound to the pointer VARIABLE (via shadow stack),
 * not the pointer VALUE. When an OOB pointer is cast to uintptr_t and stored
 * to memory, the metadata association is severed. On reload and dereference,
 * SB performs an address-based trie lookup: the pointer value falls in the
 * adjacent object's address range -> SB loads that object's metadata
 * (base, bound) -> the access appears in-bounds -> MISS.
 *
 * RSan/ASan: MISS   MixSan: DETECT   SB: MISS (address-based lookup bypass)
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char *a = (char*)malloc(16);
    char *b = (char*)malloc(16);
    if (!a || !b) return 1;
    memset(a, 0xAA, 16); memset(b, 0xBB, 16);
    uintptr_t *slot = (uintptr_t*)malloc(sizeof(uintptr_t));
    *slot = (uintptr_t)a + 32;
    char *p = (char*)(*slot);
    volatile char c = *p;
    printf("p[0]=%02x b[0]=%02x\n", (unsigned char)c, (unsigned char)b[0]);
    _exit(0);
}
