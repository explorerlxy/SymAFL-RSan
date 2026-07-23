/* T4 DS C++ tree node UAF */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct N { int k,v; N *l,*r; };
N *mk(int k,int v){N*n=(N*)malloc(sizeof(N));n->k=k;n->v=v;n->l=n->r=NULL;return n;}
int main(void) {
    N *root=mk(50,500); root->l=mk(30,300);root->r=mk(70,700);root->l->l=mk(20,200);root->l->r=mk(40,400);
    free(root->l->r);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    N *d=(N*)malloc(sizeof(N));
    N *st=root->l->r; if(st)printf("stale k=%d v=%d\n",st->k,st->v);
    _exit(0);
}
