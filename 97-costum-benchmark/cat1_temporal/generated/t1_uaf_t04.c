/* T1 UAF (thread): victim=512B dummy=512B → RSan MISS */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
static char *g = NULL; static volatile int r = 0;
void *wrk(void *a) { (void)a; while(!r); volatile char c=g[0]; printf("g[0]=%02x\n",(unsigned char)c); return NULL; }
int main(void) {
    g = (char*)malloc(512); if(!g) return 1; memset(g,0xAB,256);
    free(g);
    pthread_t th; pthread_create(&th,NULL,wrk,NULL);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    char *d = (char*)malloc(512); d[0]='D';
    r=1; pthread_join(th,NULL); _exit(0);
}
