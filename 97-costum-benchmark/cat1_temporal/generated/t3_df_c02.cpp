/* T3 DF C++ (5 holds) — same-size dummy, double-free after slot reuse */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct O { char d[56]; int id; O(int i):id(i){} };
int main(void) {
    O *o = new O(42); printf("id=%d\n", o->id); delete o;
    for (int _i = 0; _i < 12; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    O *d = new O(99);
    delete o;  // double-free: o's slot now occupied by d
    _exit(0);
}
