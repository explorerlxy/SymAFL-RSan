/* T5 C++ downcast access after delete — same-size dummy, heap access */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct A{char d[48];int age;A():age(5){}int get_age(){return age;}};
struct D:public A{int bc;D():bc(0){}int get_age(){bc++;return age+bc;}};
int main(void){A*a=new D();printf("age=%d\n",a->get_age());delete a;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
D*x=new D();x->bc=99;printf("stale=%d\n",a->get_age());_exit(0);}
