/* T4 DS hash SZ=16 r — direct stale read (no hash lookup after free) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define HS 16
typedef struct E { int k, v; struct E* n; char pad[12]; } E;
static E *ht[HS];
void ins(int k, int v) { int i = k % HS; E *e = (E*)malloc(sizeof(E)); e->k = k; e->v = v; e->n = ht[i]; ht[i] = e; }
int main(void) {
    for (int i = 0; i < HS; i++) ht[i] = NULL;
    for (int i = 0; i < 32; i++) ins(i, i*10);
    E *e = ht[16 % HS];
    while (e && e->k != 16) e = e->n;
    if (e) { printf("found k=%d v=%d\n", e->k, e->v); free(e); }
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    E *d = (E*)malloc(sizeof(E)); d->k = -1; d->v = -1;
    printf("stale k=%d v=%d\n", e->k, e->v);
    _exit(0);
}
