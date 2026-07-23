/* T5 C++ member write after delete */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct R{char b[48];int c;double rt;R():c(0),rt(0.0){}};
int main(void){R*r=new R();printf("c=%d\n",r->c);delete r;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char*x=new char[64];x[0]='D';r->c=999;printf("stale=%d\n",r->c);_exit(0);}
