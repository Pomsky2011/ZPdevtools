#include "tcc.h"

/* block scope and the integer promotions */

S16 shadow(void)
{
    S16 a;
    a = 1;
    {
        S16 a;
        a = 2;
        { S16 a; a = 3; }
        a = a + 10;
    }
    return a * 100;
}

S16 sibling_blocks(void)
{
    S16 s; s = 0;
    { S16 i; i = 5; s = s + i; }
    { S16 i; i = 7; s = s + i; }
    { S16 i; S16 j; i = 1; j = 2; s = s + i * 10 + j * 100; }
    return s;
}

S16 loop_decls(void)
{
    S16 total; S16 k;
    total = 0;
    for (k = 0; k < 3; k = k + 1) {
        S16 t;
        t = k * 2;
        total = total + t;
    }
    return total;
}

S16 shadow_param(S16 a)
{
    { S16 a; a = 50; }
    return a;
}

S16 uc_sub_neg(void) { unsigned char a; unsigned char b; a = 1; b = 2; return (a - b) < 0; }
S16 uc_sub_val(void) { unsigned char a; unsigned char b; a = 1; b = 2; return a - b; }
S16 uc_div(void) { unsigned char a; unsigned char b; a = 5; b = 200; return (a - b) / 3; }
S16 uc_shift(void) { unsigned char a; a = 200; return (a - 250) >> 1; }
S16 uc_cmp_mixed(void) { unsigned char a; S16 b; a = 3; b = -1; return a > b; }
S16 sc_arith(void) { signed char a; signed char b; a = -100; b = 100; return (a - b) / 3 + (a < b); }
S16 u16_div(void) { U16 a; a = 60000; return a / 7 + a % 7; }
S16 char_plus_int(void) { unsigned char a; a = 200; return a + 100; }
S16 char_mul(void) { unsigned char a; unsigned char b; a = 20; b = 30; return a * b; }
S16 not_char(void) { unsigned char a; a = 0; return !a + ~a + -a; }
S16 ternary_types(void) { unsigned char a; S16 b; a = 5; b = -2; return (a > 3 ? b : a) + (a < 3 ? b : a); }
S16 comma_val(void) { S16 a; a = (1, 2, 3); return a; }
S16 assign_value(void) { unsigned char c; S16 r; r = (c = 300); return r * 10 + c; }
S16 char_cmp(void) { unsigned char a; unsigned char b; a = 250; b = 5; return (a > b) * 10 + (b > a); }
S16 long_from_char(void) { unsigned char a; S32 l; a = 250; l = a; l = l * 1000L; return (S16)(l / 1000); }
S16 long_from_schar(void) { signed char a; S32 l; a = -5; l = a; return (S16)(l * 100L); }
S16 long_from_u16(void) { U16 a; S32 l; a = 50000; l = a; return (S16)(l / 100L); }
S16 long_from_s16(void) { S16 a; S32 l; a = -5; l = a; return (S16)(l * 100L); }

int main()
{
    rec(shadow());
    rec(sibling_blocks());
    rec(loop_decls());
    rec(shadow_param(7));
    rec(uc_sub_neg());
    rec(uc_sub_val());
    rec(uc_div());
    rec(uc_shift());
    rec(uc_cmp_mixed());
    rec(sc_arith());
    rec(u16_div());
    rec(char_plus_int());
    rec(char_mul());
    rec(not_char());
    rec(ternary_types());
    rec(comma_val());
    rec(assign_value());
    rec(char_cmp());
    rec(long_from_char());
    rec(long_from_schar());
    rec(long_from_u16());
    rec(long_from_s16());
    return 0;
}
