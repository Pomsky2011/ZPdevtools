/*
 * zp_timer.h - ZeroPoint hardware timer system ($D80050-$D80052).
 *
 * 8 independent timers, bit pattern 0bVHSQETAR (see CLAUDE.md "Timer System").
 * Enable a timer in CONTROL, optionally enable its IRQ in INT_ENABLE; when it
 * fires the matching STATUS bit sets and (if enabled) a CPU IRQ is raised.
 * Clear a fired timer by writing its bit back to STATUS.
 */
#ifndef ZEROPOINT_ZP_TIMER_H
#define ZEROPOINT_ZP_TIMER_H

#include "zp_types.h"
#include "zp_mmio.h"

#define ZP_TIMER_CONTROL     (ZP_IO_TIMER + 0x00)  /* $D80050 enable bits    */
#define ZP_TIMER_STATUS      (ZP_IO_TIMER + 0x01)  /* $D80051 fired flags    */
#define ZP_TIMER_INT_ENABLE  (ZP_IO_TIMER + 0x02)  /* $D80052 per-timer IRQ  */

/* Timer bits (0bVHSQETAR). */
#define ZP_TMR_VBLANK  0x80  /* ~16.67 ms  (60 Hz)            */
#define ZP_TMR_HBLANK  0x40  /* one scanline (~244 us)       */
#define ZP_TMR_1S      0x20  /* 1 second                     */
#define ZP_TMR_QUARTER 0x10  /* 1/4 second (250 ms)          */
#define ZP_TMR_EIGHTH  0x08  /* 1/8 second (125 ms)          */
#define ZP_TMR_1024    0x04  /* 1/1024 second (~977 us)      */
#define ZP_TMR_MS      0x02  /* ~1 ms                        */
#define ZP_TMR_60VBL   0x01  /* 60 V-blanks (~1 second)      */

/* Enable/disable timers (read-modify-write on CONTROL). */
ZP_INLINE void zp_timer_enable(zpu8 mask)
{
    zp_poke8(ZP_TIMER_CONTROL, (zpu8)(zp_peek8(ZP_TIMER_CONTROL) | mask));
}
ZP_INLINE void zp_timer_disable(zpu8 mask)
{
    zp_poke8(ZP_TIMER_CONTROL, (zpu8)(zp_peek8(ZP_TIMER_CONTROL) & ~mask));
}

/* Enable interrupt generation for the given timers. */
ZP_INLINE void zp_timer_irq_enable(zpu8 mask)
{
    zp_poke8(ZP_TIMER_INT_ENABLE, (zpu8)(zp_peek8(ZP_TIMER_INT_ENABLE) | mask));
}

/* Read fired flags; clear them by writing the bits back. */
ZP_INLINE zpu8 zp_timer_status(ZP_VOID)      { return zp_peek8(ZP_TIMER_STATUS); }
ZP_INLINE void zp_timer_clear(zpu8 mask)  { zp_poke8(ZP_TIMER_STATUS, mask);  }
ZP_INLINE zpbool zp_timer_fired(zpu8 mask)
{
    return (zpbool)((zp_peek8(ZP_TIMER_STATUS) & mask) != 0);
}

/* Busy-wait for a timer to fire, then clear it. */
ZP_INLINE void zp_timer_wait(zpu8 mask)
{
    while (!zp_timer_fired(mask)) { /* spin */ }
    zp_timer_clear(mask);
}

#endif /* ZEROPOINT_ZP_TIMER_H */
