/* T4 DS tree D=4 w — direct stale write */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
typedef struct T { int v; struct T *l, *r; char pad[8]; } T;
T *make(int d) { if (d<=0) return NULL; T *t=(T*)malloc(sizeof(T)); t->v=d; t->l=make(d-1); t->r=make(d-1); return t; }
int main(void) {
    T *root = make(4);
    T *n = root->l->r;
    free(n);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    T *d = (T*)malloc(sizeof(T)); d->v = 99;
    n->v = 0xCD;
    printf("wrote stale=%d\n", n->v);
    _exit(0);
}
