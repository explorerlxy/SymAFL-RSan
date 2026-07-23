/* T5 C++ thread member after delete */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
struct D{char b[56];int c;D():c(0){}void inc(){c++;}};
static D*g=NULL;static volatile int r=0;
void*wrk(void*a){(void)a;while(!r);g->inc();printf("wrk=%d\n",g->c);return NULL;}
int main(void){g=new D();printf("c=%d\n",g->c);pthread_t th;pthread_create(&th,NULL,wrk,NULL);delete g;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char*x=new char[64];x[0]='D';r=1;pthread_join(th,NULL);_exit(0);}
