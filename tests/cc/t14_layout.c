#include "tcc.h"
struct C3 { unsigned char a; unsigned char b; unsigned char c; };
struct M { unsigned char a; S16 b; unsigned char c; };
struct W { struct C3 x; unsigned char y; };
int main() {
    struct C3 t[3];
    struct M m;
    struct W w;
    rec(sizeof(struct C3)); rec(sizeof(struct M)); rec(sizeof(struct W)); rec(sizeof(t));
    rec((S16)((char*)&m.b - (char*)&m)); rec((S16)((char*)&m.c - (char*)&m));
    rec((S16)((char*)&w.y - (char*)&w));
    return 0;
}
