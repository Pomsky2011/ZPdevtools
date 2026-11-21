/*
 * zp_serial.h - the debug serial port ($D8003C-$D8003E), a UART on a dev unit.
 *
 * zeropoint_sdl --dev-unit makes the console a dev unit: bytes written here reach the host (terminal, file or TCP socket)
 * and bytes the host types can be read back. On a retail unit nothing drains the transmit queue, so the helpers below do
 * nothing there (a debug print must not hang a shipping program once the 255-byte queue fills). See the emulator's
 * docs/debug-protocol.md.
 */
#ifndef ZEROPOINT_ZP_SERIAL_H
#define ZEROPOINT_ZP_SERIAL_H

#include "zp_types.h"
#include "zp_mmio.h"

#define ZP_SER_STATUS   (ZP_IO_PORTS + 0x0C)  /* $D8003C */
#define ZP_SER_DATA     (ZP_IO_PORTS + 0x0D)  /* $D8003D */
#define ZP_SER_CTRL     (ZP_IO_PORTS + 0x0E)  /* $D8003E */

#define ZP_SER_RX_READY 0x01  /* a received byte is waiting                      */
#define ZP_SER_TX_EMPTY 0x02  /* the transmit queue has room                     */
#define ZP_SER_OVERRUN  0x04  /* dev unit: a received byte was dropped           */
#define ZP_SER_TX_DONE  0x08  /* dev unit: everything queued has gone out        */
#define ZP_SER_CLEAR    0x01  /* SER_CTRL: clear both queues                     */
#define ZP_SER_CLEAR_OVERRUN 0x02  /* SER_CTRL: clear OVERRUN (dev unit)         */

/* 1 on a dev unit. The latch is set by the host; no program can write it. */
ZP_INLINE zpbool zp_is_dev_unit(ZP_VOID)
{
    return (zpbool)((zp_peek8(ZP_SYS_DEVMODE) & ZP_SYS_DEVUNIT) != 0);
}

/* Send one byte, waiting for room in the queue (115200 baud: ~1476 CPU cycles per byte). No-op on a retail unit. */
ZP_INLINE void zp_serial_putc(zpu8 c)
{
    if (!zp_is_dev_unit()) return;
    while ((zp_peek8(ZP_SER_STATUS) & ZP_SER_TX_EMPTY) == 0) { }
    zp_poke8(ZP_SER_DATA, c);
}

/* Send a NUL-terminated string held in RAM. (The compilers do not take string literals as pointers: copy one into an array.) */
ZP_INLINE void zp_serial_puts(char *s)
{
    while (*s) {
        zp_serial_putc((zpu8)*s);
        s++;
    }
}

/* Wait until every queued byte has gone out (before halting, say). No-op on a retail unit. */
ZP_INLINE void zp_serial_flush(ZP_VOID)
{
    if (!zp_is_dev_unit()) return;
    while ((zp_peek8(ZP_SER_STATUS) & ZP_SER_TX_DONE) == 0) { }
}

/* 1 if a byte typed on the host is waiting. */
ZP_INLINE zpbool zp_serial_ready(ZP_VOID)
{
    return (zpbool)((zp_peek8(ZP_SER_STATUS) & ZP_SER_RX_READY) != 0);
}

/* The next typed byte (wait with zp_serial_ready() first; reads 0 when nothing is waiting). */
ZP_INLINE zpu8 zp_serial_getc(ZP_VOID)
{
    return zp_peek8(ZP_SER_DATA);
}

#endif /* ZEROPOINT_ZP_SERIAL_H */
