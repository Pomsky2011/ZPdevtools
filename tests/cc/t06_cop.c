/* EXPECT: 5 1000 32767 8192 13 1000 23170 23170 7 */
/* NEEDS: boot */
/* Every COP signature through the SDK: the typed wrappers, the three-register zp_cop3 and the 4-argument
 * zp_cop32 (whose result both compilers used to destroy when they popped the stack argument). */
#include <zeropoint/zeropoint.h>
#include "tcc.h"

int main()
{
    U32 sc;
    rec(zp_hypot(3, 4));                          /* 5 */
    rec(zp_sqrt32(1000000UL));                    /* 1000 */
    rec(zp_sin(0x4000));                          /* 32767 */
    rec(zp_atan2(1, 1));                          /* 8192 */
    rec((S16)zp_cop(0xF6, 0, 5, 12));             /* hypot(5, 12) = 13: 4 arguments, one on the stack */
    rec((S16)zp_cop3(0xF5, 0x4240, 0x000F));      /* 1000 */
    sc = zp_sincos(0x2000);
    rec32(sc);                                    /* sin 45 deg = cos 45 deg = 23170 */
    rec((S16)zp_cop3(0xF8, 7, 0));                /* reserved $F8 hands A back */
    return 0;
}
