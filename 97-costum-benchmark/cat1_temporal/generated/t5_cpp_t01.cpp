/* T5 C++ thread member access after delete — non-virtual, same-size dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
struct O{char d[56];int id;O(int i):id(i){}int g(){return id;}};
static O*g=NULL;static volatile int r=0;
void*wrk(void*a){(void)a;while(!r);printf("wrk=%d\n",g->id);return NULL;}
int main(void){g=new O(42);printf("id=%d\n",g->id);pthread_t th;pthread_create(&th,NULL,wrk,NULL);delete g;
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
char*x=new char[sizeof(O)];x[0]='D';r=1;pthread_join(th,NULL);_exit(0);}
