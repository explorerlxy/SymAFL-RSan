/* T1 C++ array UAF: delete[] + UAF — same-size dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Rec { int id; char name[60]; };
int main(void) {
    Rec *arr = new Rec[3];
    for (int i = 0; i < 3; i++) arr[i].id = i + 100;
    printf("arr[1].id=%d\n", arr[1].id);
    delete[] arr;
    for (int i = 0; i < 10; i++) { volatile char *t = new char[32U*1024*1024]; if (t) { t[0] = (char)i; delete[] t; } }
    Rec *d = new Rec[3]();
    d[0].id = -1;
    printf("stale=%d\n", arr[1].id);
    _exit(0);
}
