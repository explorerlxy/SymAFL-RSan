/* T1 UAF (C++): victim=32B dummy=32B exhaust=10x32MB → RSan MISS */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
class W { public: char data[24]; int id; W(int i):id(i){} int g(){return id;} };
int main(void) {
    W *w = new W(42); printf("id=%d\n", w->g()); delete w;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = new char[32U*1024*1024];
        if (_t) { _t[0] = (char)_i; delete[] _t; }
    }
    char *d = new char[32]; d[0] = 'D';
    printf("stale=%d\n", w->g());  // virtual call through dangling ptr
    _exit(0);
}
