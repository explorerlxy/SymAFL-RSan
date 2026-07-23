/* T5 C++ realloc chain UAF (new+copy+delete simulates realloc) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(void) {
    int n=5; char *ptrs[5];
    ptrs[0]=new char[64]; memset(ptrs[0],0x11,64);
    for(int i=1;i<n;i++){ ptrs[i]=new char[64+i*4096]; memcpy(ptrs[i],ptrs[i-1],64+(i-1)*4096); delete[] ptrs[i-1]; }
    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }
    char *d=new char[64]; d[0]='D';
    volatile char c=ptrs[0][0]; printf("ptrs[0][0]=%02x\n",(unsigned char)c); _exit(0);
}
