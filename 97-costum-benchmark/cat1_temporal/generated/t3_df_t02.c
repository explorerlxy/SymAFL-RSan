/* T3 DF thread (5 holds) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
static char *g=NULL; static volatile int r=0;
void *wrk(void *a){ (void)a; while(!r); free(g); printf("df done\n"); return NULL; }
int main(void) {
    g=(char*)malloc(64); if(!g) return 1; memset(g,0xAB,64);
    pthread_t th; pthread_create(&th,NULL,wrk,NULL);
    free(g);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    char *_hold[5];
    for (int _i = 0; _i < 5; _i++) { _hold[_i]=(char*)malloc(64); if(_hold[_i]) _hold[_i][0]=(char)_i; }
    r=1; pthread_join(th,NULL); _exit(0);
}
