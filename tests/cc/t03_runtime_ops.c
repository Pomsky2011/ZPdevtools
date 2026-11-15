#include "tcc.h"

/* every operation behind a call, so the compiler can't fold it */
S16 add(S16 a, S16 b) { return a + b; }
S16 sub(S16 a, S16 b) { return a - b; }
S16 mul(S16 a, S16 b) { return a * b; }
S16 dv(S16 a, S16 b) { return a / b; }
S16 md(S16 a, S16 b) { return a % b; }
S16 band(S16 a, S16 b) { return a & b; }
S16 bor(S16 a, S16 b) { return a | b; }
S16 bxor(S16 a, S16 b) { return a ^ b; }
S16 shl(S16 a, S16 b) { return a << b; }
S16 sar(S16 a, S16 b) { return a >> b; }
U16 shr(U16 a, U16 b) { return a >> b; }
U16 udv(U16 a, U16 b) { return a / b; }
U16 umd(U16 a, U16 b) { return a % b; }
S16 slt(S16 a, S16 b) { return a < b; }
S16 sle(S16 a, S16 b) { return a <= b; }
S16 sgt(S16 a, S16 b) { return a > b; }
S16 sge(S16 a, S16 b) { return a >= b; }
S16 seq(S16 a, S16 b) { return a == b; }
S16 sne(S16 a, S16 b) { return a != b; }
S16 ult(U16 a, U16 b) { return a < b; }
S16 ule(U16 a, U16 b) { return a <= b; }
S16 ugt(U16 a, U16 b) { return a > b; }
S16 uge(U16 a, U16 b) { return a >= b; }
S16 neg(S16 a) { return -a; }
S16 inv(S16 a) { return ~a; }
S16 lnot(S16 a) { return !a; }

int main()
{
    rec(add(-3, 10));
    rec(sub(3, 10));
    rec(mul(-6, 7));
    rec(mul(100, 30));
    rec(dv(-8, 2));
    rec(dv(7, -2));
    rec(dv(-7, -2));
    rec(md(-7, 3));
    rec(md(7, -3));
    rec(md(-7, -3));
    rec(md(8, 3));
    rec(band(0x0F0F, 0x00FF));
    rec(bor(0x0F00, 0x00F0));
    rec(bxor(0x0FF0, 0x00FF));
    rec(shl(3, 4));
    rec(shl(1, 0));
    rec(sar(-16, 2));
    rec(sar(0x4000, 3));
    rec(shr(0x8000, 15));
    rec(shr(0xFFFF, 4));
    rec(udv(60000, 7));
    rec(umd(60000, 7));
    rec(slt(-5, 3));
    rec(slt(3, -5));
    rec(slt(-5, -5));
    rec(slt(30000, -30000));
    rec(sle(-5, -5));
    rec(sle(2, -1));
    rec(sgt(-1, -2));
    rec(sgt(-2, -1));
    rec(sge(-2, -2));
    rec(sge(-3, -2));
    rec(sge(30000, -30000));
    rec(seq(-1, -1));
    rec(sne(-1, 1));
    rec(ult(1, 2));
    rec(ult(2, 1));
    rec(ult(1, 65535));
    rec(ult(65535, 1));
    rec(ule(7, 7));
    rec(ugt(65535, 1));
    rec(ugt(1, 65535));
    rec(uge(40000, 40000));
    rec(uge(40000, 50000));
    rec(neg(5));
    rec(neg(-5));
    rec(inv(0));
    rec(inv(0x00FF));
    rec(lnot(0));
    rec(lnot(7));
    return 0;
}
