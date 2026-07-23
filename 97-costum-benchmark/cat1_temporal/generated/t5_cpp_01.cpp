/* T5 C++ member access after delete — same-size dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Base { char d[56]; int id; Base(int i):id(i){} };
int main(void) { Base *o=new Base(42); printf("id=%d\n",o->id); delete o;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char *x=new char[sizeof(Base)];x[0]='D'; printf("stale=%d\n",o->id); _exit(0); }
