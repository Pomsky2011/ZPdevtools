#include "tcc.h"

S32 ladd(S32 a, S32 b) { return a + b; }
S32 lsub(S32 a, S32 b) { return a - b; }
S32 lmul(S32 a, S32 b) { return a * b; }
S32 ldv(S32 a, S32 b) { return a / b; }
S32 lmd(S32 a, S32 b) { return a % b; }
U32 ludv(U32 a, U32 b) { return a / b; }
U32 lumd(U32 a, U32 b) { return a % b; }
U32 land(U32 a, U32 b) { return a & b; }
U32 lor(U32 a, U32 b) { return a | b; }
U32 lxor(U32 a, U32 b) { return a ^ b; }
U32 lshl(U32 a, S16 n) { return a << n; }
U32 lshr(U32 a, S16 n) { return a >> n; }
S32 lsar(S32 a, S16 n) { return a >> n; }
S32 lneg(S32 a) { return -a; }
U32 lnot(U32 a) { return ~a; }
S16 slt(S32 a, S32 b) { return a < b; }
S16 sle(S32 a, S32 b) { return a <= b; }
S16 sgt(S32 a, S32 b) { return a > b; }
S16 sge(S32 a, S32 b) { return a >= b; }
S16 seq(S32 a, S32 b) { return a == b; }
S16 sne(S32 a, S32 b) { return a != b; }
S16 ult(U32 a, U32 b) { return a < b; }
S16 ugt(U32 a, U32 b) { return a > b; }
S16 uge(U32 a, U32 b) { return a >= b; }
S32 widen(S16 a) { return a; }
U32 uwiden(U16 a) { return a; }
S16 narrow(S32 a) { return (S16)a; }
S32 mixed(S32 a, S16 b) { return a + b; }
S32 mixed2(S16 a, S32 b) { return a - b; }
S32 locals(S32 a) { S32 x; S32 y; x = a + 1; y = x + x; return y; }
U32 pairargs(S16 a, U32 b, S16 c) { return b + a + c; }
S32 manyargs(S32 a, S32 b, S32 c) { return a + b + b + c + c + c; }
S32 callnest(S32 a) { return ladd(ladd(a, 1), ladd(a, 2)); }
S32 tern(S16 c, S32 a, S32 b) { return c ? a : b; }
S32 incdec(S32 a) { S32 x; x = a; x++; ++x; x--; x++; return x; }
S32 postval(S32 a) { S32 x; S32 y; x = a; y = x++; return y + y + x; }
S16 truth(S32 a) { if (a) return 1; return 0; }
S16 lnotv(S32 a) { return !a; }
S16 logic(S32 a, S32 b) { return a && b; }
S16 logor(S32 a, S32 b) { return a || b; }
U32 addrmix(U32 base, U16 off) { return base + off; }
S16 b2(U32 v) { return (S16)((v >> 16) & 0xFF); }
S32 loop32(S16 n) { S32 s; S16 i; s = 0; for (i = 0; i < n; i = i + 1) s = s + 100000; return s; }

int main()
{
    rec32(ladd(100000, 23456));
    rec32(ladd(0x7FFF, 1));
    rec32(ladd(-1, 1));
    rec32(lsub(5, 100000));
    rec32(lmul(300, 1000));
    rec32(lmul(-300, 1000));
    rec32(lmul(65536, 3));
    rec32(ldv(1000000, 7));
    rec32(ldv(-1000000, 7));
    rec32(ldv(1000000, -7));
    rec32(lmd(1000000, 7));
    rec32(lmd(-1000000, 7));
    rec32(ludv(4000000000u, 7));
    rec32(lumd(4000000000u, 7));
    rec32(land(0xF0F0F0F0u, 0x00FF00FFu));
    rec32(lor(0xF0F0F0F0u, 0x000F000Fu));
    rec32(lxor(0xFFFF0000u, 0x00FFFF00u));
    rec32(lshl(1, 20));
    rec32(lshl(0x12345, 4));
    rec32(lshr(0x80000000u, 31));
    rec32(lshr(0xFFFFFFFFu, 8));
    rec32(lsar(-65536, 4));
    rec32(lsar(0x40000000, 14));
    rec32(lneg(100000));
    rec32(lneg(-7));
    rec32(lnot(0x0000FFFFu));
    rec(slt(-100000, 5));
    rec(slt(5, -100000));
    rec(slt(100000, 100001));
    rec(sle(7, 7));
    rec(sgt(100000, 99999));
    rec(sgt(-5, -100000));
    rec(sge(-100000, -100000));
    rec(seq(65536, 65536));
    rec(seq(65536, 0));
    rec(sne(65536, 0));
    rec(ult(1, 4000000000u));
    rec(ult(4000000000u, 1));
    rec(ugt(4000000000u, 5));
    rec(uge(70000, 70000));
    rec32(widen(-5));
    rec32(widen(1234));
    rec32(uwiden(60000));
    rec(narrow(0x12345678));
    rec(narrow(-1));
    rec32(mixed(100000, -3));
    rec32(mixed2(7, 100000));
    rec32(locals(100000));
    rec32(pairargs(1, 100000, 2));
    rec32(manyargs(1, 2, 3));
    rec32(callnest(100000));
    rec32(tern(1, 70000, 5));
    rec32(tern(0, 70000, 5));
    rec32(incdec(65535));
    rec32(postval(100));
    rec(truth(0));
    rec(truth(65536));
    rec(truth(1));
    rec(lnotv(0));
    rec(lnotv(65536));
    rec(logic(65536, 1));
    rec(logic(65536, 0));
    rec(logor(0, 65536));
    rec(logor(0, 0));
    rec32(addrmix(0x800300u, 0x20));
    rec(b2(0xA5123456u));
    rec32(loop32(5));
    return 0;
}
