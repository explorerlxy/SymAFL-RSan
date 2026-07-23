/* T5 C++ template UAF */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
template<typename T>class Box{T val;char pad[48];public:Box(T v):val(v){}T g(){return val;}virtual~Box(){}};
int main(void){Box<int>*b=new Box<int>(42);printf("v=%d\n",b->g());delete b;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char*x=new char[64];x[0]='D';printf("stale=%d\n",b->g());_exit(0);}
