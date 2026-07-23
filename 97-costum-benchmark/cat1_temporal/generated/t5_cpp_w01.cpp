/* T5 C++ member write after delete — non-virtual, same-size dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct W{char d[56];int id;W(int i):id(i){}int g(){return id;}void s(int i){id=i;}};
int main(void){W*w=new W(42);printf("id=%d\n",w->g());delete w;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char*x=new char[sizeof(W)];x[0]='D';w->s(99);printf("stale=%d\n",w->g());_exit(0);}
