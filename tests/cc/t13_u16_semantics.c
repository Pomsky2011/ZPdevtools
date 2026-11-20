/* EXPECT: 1 1 1 0 1 1 1 */
#include "tcc.h"

/* 16-bit int: unsigned short/int do not promote to a wider signed type, so these differ from a 32-bit host and carry
   their expected values in the EXPECT line above instead of coming from a gcc run. */

S16 sub_wraps(void) { U16 a; U16 b; a = 1; b = 2; return (a - b) > 0; }
S16 mixed_compare(void) { U16 a; S16 b; a = 40000; b = -1; return a < b; }
S16 zero_minus_one(void) { U16 a; a = 0; return (a - 1) == 65535; }
S16 never_negative(void) { U16 a; a = 0; return (a - 1) < 0; }
S16 shr_logical(void) { U16 a; a = 0x8000; return (a >> 15); }
S16 neg_vs_unsigned(void) { U16 a; a = 1; return -1 > a; }
S16 big_literal(void) { return 40000 > 100; }

int main()
{
    rec(sub_wraps());
    rec(mixed_compare());
    rec(zero_minus_one());
    rec(never_negative());
    rec(shr_logical());
    rec(neg_vs_unsigned());
    rec(big_literal());
    return 0;
}
