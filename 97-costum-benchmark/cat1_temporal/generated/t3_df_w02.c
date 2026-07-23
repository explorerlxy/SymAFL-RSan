/* T3 DF write (5 holds) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(void) {
    char *p = (char*)malloc(64); if(!p) return 1; memset(p,0xAB,64);
    free(p);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    char *_hold[5];
    for (int _i = 0; _i < 5; _i++) { _hold[_i]=(char*)malloc(64); if(_hold[_i]) _hold[_i][0]=(char)_i; }
    p[0] = 0xCD; free(p); printf("done\n"); _exit(0);
}
