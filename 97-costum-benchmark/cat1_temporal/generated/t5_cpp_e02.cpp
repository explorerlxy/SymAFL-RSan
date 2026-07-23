/* T5 C++ deep inheritance UAF — non-virtual, same-size dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct L0{char d0[48];int v0;L0():v0(10){}int g(){return v0;}};
struct L1:public L0{int v1;L1():v1(20){}int g(){return v1;}};
struct L2:public L1{int v2;L2():v2(30){}int g(){return v2;}};
struct L3:public L2{int v3;L3():v3(40){}int g(){return v3;}};
struct L4:public L3{int v4;L4():v4(50){}int g(){return v4;}};
int main(void){L0*o=new L4();printf("g=%d\n",o->g());delete o;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char*x=new char[sizeof(L4)];x[0]='D';printf("stale=%d\n",o->g());_exit(0);}
