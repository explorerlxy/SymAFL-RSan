/* T1 C++ vector iterator UAF: realloc frees old data, exhaustion, UAF via iterator */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Vec { int *d; size_t cap, sz; Vec() : d(NULL), cap(0), sz(0) {} };
void vp(Vec *v, int x) {
    if (v->sz >= v->cap) { size_t nc = v->cap ? v->cap*2 : 16; int *nd = (int*)malloc(nc*4); for (size_t i=0;i<v->sz;i++) nd[i]=v->d[i]; free(v->d); v->d=nd; v->cap=nc; }
    v->d[v->sz++] = x;
}
int main(void) {
    Vec v; vp(&v,10); vp(&v,20); vp(&v,30); vp(&v,40); vp(&v,50); vp(&v,60); vp(&v,70); vp(&v,80);
    int *it = v.d;  // save iterator before realloc
    printf("*it=%d\n", *it);
    for (int i = 0; i < 100; i++) vp(&v, i);  // force realloc → old d freed
    for (int i = 0; i < 10; i++) { volatile char *t = (char*)malloc(32U*1024*1024); if (t) { t[0] = (char)i; free((void*)t); } }
    int *dummy = (int*)malloc(64); dummy[0] = 0xDD;
    printf("stale=%d\n", *it);  // UAF via stale iterator
    _exit(0);
}
