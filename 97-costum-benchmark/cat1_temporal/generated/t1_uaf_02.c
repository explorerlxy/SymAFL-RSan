/* T1 UAF (read): v=32B d=32B off=0 exhaust=10x32MB → RSan MISS */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(void) {
    // Step 1: allocate victim
    char *p = (char*)malloc(32);
    if (!p) return 1;
    memset(p, 0xAB, 32);
    // Step 2: free → enters quarantine
    free(p);
    // Step 3: exhaust quarantine (10 x 32MB = 320MB)
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    // Step 4: dummy allocation — same size class, possibly different endaddr
    char *d = (char*)malloc(32);
    if (!d) return 1;
    memset(d, 0xCC, 32);
    // Step 5: UAF via dangling pointer at offset 0
    volatile char c = p[0]; printf("p[%d]=%02x d[0]=%02x\\n", 0, (unsigned char)c, (unsigned char)d[0]);
    // 320MB > 256MB quarantine → victim evicted → RSan MISS
    _exit(0);
}
