/* T3 DF (15 holds before DF) */
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
    char *_hold[15];
    for (int _i = 0; _i < 15; _i++) { _hold[_i]=(char*)malloc(64); if(_hold[_i]) _hold[_i][0]=(char)_i; }
    free(p); printf("done\n"); _exit(0);
}
