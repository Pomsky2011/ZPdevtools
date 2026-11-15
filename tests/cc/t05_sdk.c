/* EXPECT: 90 4660 52 18 7 1 3572 21 5 */
/* The real C SDK on the real peripherals: 32-bit bus addresses as arguments (A:X), 32-bit loop arithmetic, DMA. */
#include <zeropoint/zeropoint.h>
#include "tcc.h"

int main()
{
    U16 sum;
    U32 i;
    zpu16 xfers;

    zp_poke8(0x80D000UL, 0x5A);
    rec(zp_peek8(0x80D000UL));            /* 90 */
    zp_poke16(0x80D002UL, 0x1234);
    rec(zp_peek16(0x80D002UL));           /* 4660 */
    rec(zp_peek8(0x80D002UL));            /* 52 */
    rec(zp_peek8(0x80D003UL));            /* 18 */
    zp_poke8(ZP_RAM_BASE + 0xD100UL, 7);
    rec(zp_peek8(0x80D100UL));            /* 7 */

    for (i = 0; i < 600; i = i + 1)
        zp_poke8(0x80D200UL + i, (zpu8)(i & 0xFF));
    xfers = zp_dma_load(0x80D200UL, 0x80D600UL, 600UL, 0);
    rec((S16)xfers);                       /* 600 rounds up to 768: one transfer */
    sum = 0;
    for (i = 0; i < 600; i = i + 1)
        sum = sum + zp_peek8(0x80D600UL + i);
    rec((S16)sum);                         /* 69108 mod 65536 = 3572 */
    zp_dma_fill(0x80DA00UL, 0x15, 256UL, 1);
    rec(zp_peek8(0x80DA00UL));             /* 21 */
    rec(zp_peek8(0x80DAFFUL) - 16);        /* 21 - 16 = 5 */
    return 0;
}
