#include "tcc.h"

/* char / short and casts.  Only explicitly signed/unsigned types are used (plain char's signedness is
   implementation-defined), and only values whose conversion is defined, so the host gcc is the reference. */

S16 uc_wrap(S16 n) { unsigned char c; c = n; return c; }
S16 sc_wrap(S16 n) { signed char c; c = n; return c; }
S16 uc_inc(S16 n) { unsigned char c; c = 250; for (n = n; n > 0; n = n - 1) c = c + 1; return c; }
S16 sc_inc(S16 n) { signed char c; c = 125; for (n = n; n > 0; n = n - 1) c = c + 1; return c; }
S16 cast_uc(S16 n) { return (unsigned char)n; }
S16 cast_sc(S16 n) { return (signed char)n; }
S16 uc_cmp(S16 n) { unsigned char a; unsigned char b; a = n; b = 200; return a > b; }
S16 sc_cmp(S16 n) { signed char a; signed char b; a = n; b = -5; return a > b; }
S16 uc_add(S16 a, S16 b) { unsigned char x; unsigned char y; x = a; y = b; return x + y; }
S16 sc_add(S16 a, S16 b) { signed char x; signed char y; x = a; y = b; return x + y; }
S16 uc_mul(S16 a, S16 b) { unsigned char x; unsigned char y; x = a; y = b; return x * y; }
S16 uc_shr(S16 a, S16 b) { unsigned char x; x = a; return x >> b; }
S16 sc_shr(S16 a, S16 b) { signed char x; x = a; return x >> b; }
S16 sh_wrap(S16 n) { short s; s = n; return s; }
S16 us_val(S16 n) { unsigned short u; u = n; return u == 40000; }
S16 u16_big(S16 n) { U16 u; u = n; return u > 32767; }
S16 u16_div(S16 n) { U16 u; u = n; return u / 3; }
S16 u16_mod(S16 n) { U16 u; u = n; return u % 1000; }
S16 u16_shr(S16 n) { U16 u; u = n; return u >> 4; }
S16 u16_lt(S16 a, S16 b) { U16 x; U16 y; x = a; y = b; return x < y; }
S16 pass_narrow(unsigned char c, signed char s) { return c * 2 + s; }

int main()
{
    rec(uc_wrap(300));    rec(uc_wrap(255));    rec(uc_wrap(256));    rec(uc_wrap(-1));
    rec(sc_wrap(127));    rec(sc_wrap(128));    rec(sc_wrap(200));    rec(sc_wrap(-129));
    rec(uc_inc(3));       rec(sc_inc(3));
    rec(cast_uc(513));    rec(cast_sc(255));    rec(cast_sc(-3));     rec(cast_uc(-3));
    rec(uc_cmp(250));     rec(uc_cmp(100));     rec(sc_cmp(-1));      rec(sc_cmp(-100));
    rec(uc_add(200, 100)); rec(sc_add(100, 100)); rec(sc_add(-100, -100));
    rec(uc_mul(20, 30));
    rec(uc_shr(240, 3));  rec(sc_shr(-16, 2));  rec(sc_shr(100, 2));
    rec(sh_wrap(12345));  rec(sh_wrap(-1));
    rec(us_val(-25536));  rec(u16_big(-1));     rec(u16_big(100));
    rec(u16_div(-1));     rec(u16_mod(-1));     rec(u16_shr(-1));     rec(u16_shr(40000));
    rec(u16_lt(1, -1));   rec(u16_lt(-1, 1));   rec(u16_lt(-2, -1));
    rec(pass_narrow(200, -3));
    return 0;
}
