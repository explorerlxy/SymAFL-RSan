/* T1 C++ member func UAF: non-virtual method on dangling this — same-type dummy */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Logger {
    char buf[56]; int wr;
    Logger() : wr(0) { buf[0] = 0; }
    void log(const char *m) { int n = strlen(m); if (wr+n < 55) { strcpy(buf+wr, m); wr += n; } }
    void flush() { printf("LOG[%d]: %s\n", wr, buf); wr = 0; }
};
int main(void) {
    Logger *log = new Logger(); log->log("hello"); log->flush(); delete log;
    for (int i = 0; i < 10; i++) { volatile char *t = new char[32U*1024*1024]; if (t) { t[0] = (char)i; delete[] t; } }
    Logger *d = new Logger(); d->log("dummy");
    log->log("uaf!"); log->flush();
    _exit(0);
}
