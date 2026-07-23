/* T5 C++ member access after delete — same-size dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct D { char b[56]; int v; double s; D():v(99),s(3.14){} void p(){printf("v=%d s=%.2f\n",v,s);} };
int main(void) { D *d=new D(); d->p(); delete d;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char *x=new char[sizeof(D)];x[0]='D'; d->p(); _exit(0); }
