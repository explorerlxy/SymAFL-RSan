/* T4 DS list L=8 r — direct stale read */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
typedef struct N { int v; struct N* n; char pad[16]; } N;
int main(void) {
    N *head = NULL;
    for (int i = 0; i < 8; i++) { N *nn = (N*)malloc(sizeof(N)); nn->v = i; nn->n = head; head = nn; }
    N *mid = head;
    for (int i = 0; i < 4; i++) mid = mid->n;
    free(mid);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    N *d = (N*)malloc(sizeof(N)); d->v = 99;
    printf("stale=%d\n", mid->v);
    _exit(0);
}
