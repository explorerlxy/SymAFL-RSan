/* T4 DS C++ vector iterator UAF */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Vec { int *data; int cap, len; };
void vp(Vec *v, int x) { if(v->len>=v->cap){int nc=v->cap?v->cap*2:4;int*nd=(int*)malloc(nc*4);for(int i=0;i<v->len;i++)nd[i]=v->data[i];free(v->data);v->data=nd;v->cap=nc;}v->data[v->len++]=x; }
int main(void) {
    Vec v={NULL,0,0}; for(int i=0;i<10;i++)vp(&v,i*10);
    int *old=v.data; for(int i=0;i<20;i++)vp(&v,i*100);
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    char *d=(char*)malloc(64);d[0]='D';
    printf("stale=%d\n",old[0]);_exit(0);
}
