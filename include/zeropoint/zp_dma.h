/*
 * zp_dma.h - ZeroPoint DMA controller: config builders and blob loaders.
 *
 * The DMA controller (32 MHz, 16 channels, max 2 concurrent) is programmed by
 * streaming a 9-byte configuration to $D80021 after resetting the index with a
 * write to $D80020.  A single transfer moves 256..1024 bytes:
 *
 *     total = ((size_bits + 1) << 8) * multiplier      (must be <= 1024)
 *
 * So 256 bytes is the smallest transfer and the granularity is 256 bytes.
 * The blob loaders below split larger regions into <=1024-byte transfers and
 * round the tail up to the next 256-byte boundary (pad your assets to 256).
 *
 * 9-byte config layout (little-endian addresses):
 *   [0] SS MM CCCC : size_bits<<6 | mode<<4 | channel
 *   [1..3]         : 24-bit source address
 *   [4..6]         : 24-bit target address
 *   [7]            : size multiplier
 *   [8]            : interrupt / trigger byte
 */
#ifndef ZEROPOINT_ZP_DMA_H
#define ZEROPOINT_ZP_DMA_H

#include "zp_types.h"
#include "zp_mmio.h"

/* Transfer modes (see include/dma.h DMAMode in the emulator). */
#define ZP_DMA_COPY         0  /* copy [src..] -> [dst..]          */
#define ZP_DMA_CONST_COPY   1  /* fill [dst..] with constant byte  */
#define ZP_DMA_REPEAT       2  /* repeat pattern -> [dst..]        */
#define ZP_DMA_CONST_REPEAT 3  /* repeat constant -> [dst..]       */

/* Channel state values reported by ZP_DMA_CHAN(n) (emulator DMAState). */
#define ZP_DMA_ST_IDLE      0
#define ZP_DMA_ST_COMPLETE  4

#define ZP_DMA_MAX_XFER     1024U  /* bytes per single transfer */
#define ZP_DMA_UNIT         256U   /* transfer granularity      */

/*
 * Kick a single DMA transfer by streaming the 9-byte config directly to the
 * controller (no buffer, no array subscripting - so it lowers on both zpcc and
 * def88186cc).  The 9th byte auto-fires the queue.
 *   size_bits : 0..3  -> base block of (size_bits+1)*256 bytes
 *   mult      : 1..255 (block * mult must be <= 1024)
 *   irq       : trigger byte written last (non-zero requests completion IRQ)
 */
ZP_INLINE void zp_dma_kick(zpu8 mode, zpu8 chan, zpaddr src, zpaddr dst,
                           zpu8 size_bits, zpu8 mult, zpu8 irq)
{
    zp_poke8(ZP_DMA_RESET, 0);          /* reset the config write index */
    zp_poke8(ZP_DMA_CONFIG,
             (zpu8)(((size_bits & 3) << 6) | ((mode & 3) << 4) | (chan & 0x0F)));
    zp_poke8(ZP_DMA_CONFIG, ZP_B0(src));
    zp_poke8(ZP_DMA_CONFIG, ZP_B1(src));
    zp_poke8(ZP_DMA_CONFIG, ZP_B2(src));
    zp_poke8(ZP_DMA_CONFIG, ZP_B0(dst));
    zp_poke8(ZP_DMA_CONFIG, ZP_B1(dst));
    zp_poke8(ZP_DMA_CONFIG, ZP_B2(dst));
    zp_poke8(ZP_DMA_CONFIG, mult);
    zp_poke8(ZP_DMA_CONFIG, irq);       /* auto-fires the transfer */
}

/* True while the given channel is still configuring/transferring. */
ZP_INLINE zpbool zp_dma_busy(zpu8 chan)
{
    zpu8 st;
    if (chan > 11) return ZP_FALSE;      /* only ch0-11 are status-visible */
    st = zp_peek8(ZP_DMA_CHAN(chan));
    return (zpbool)(st != ZP_DMA_ST_IDLE && st != ZP_DMA_ST_COMPLETE);
}

/* Spin until the channel returns to idle. */
ZP_INLINE void zp_dma_wait(zpu8 chan)
{
    while (zp_dma_busy(chan)) { /* busy-wait */ }
}

/*
 * Split (src -> dst, len bytes) into <=1024-byte DMA copies on one channel,
 * waiting for each so a single channel is never over-queued.  len is rounded
 * up to a 256-byte multiple.  Returns the number of transfers issued.
 */
ZP_INLINE zpu16 zp_dma_load(zpaddr src, zpaddr dst, zpu32 len, zpu8 chan)
{
    zpu16 xfers = 0;
    zpu32 done = 0;
    zpu32 total = (len + (ZP_DMA_UNIT - 1)) & ~((zpu32)(ZP_DMA_UNIT - 1));

    while (done < total) {
        zpu32 chunk = total - done;
        zpu8  units;
        if (chunk > ZP_DMA_MAX_XFER) chunk = ZP_DMA_MAX_XFER;
        units = (zpu8)(chunk / ZP_DMA_UNIT);      /* 1..4 -> 256..1024 */
        zp_dma_wait(chan);
        zp_dma_kick(ZP_DMA_COPY, chan, src + done, dst + done,
                    (zpu8)(units - 1), 1, 0);
        done  += chunk;
        xfers += 1;
    }
    zp_dma_wait(chan);
    return xfers;
}

/* Fill dst..dst+len with a constant byte via ConstRepeat DMA. */
ZP_INLINE void zp_dma_fill(zpaddr dst, zpu8 value, zpu32 len, zpu8 chan)
{
    zpu32 done = 0;
    zpu32 total = (len + (ZP_DMA_UNIT - 1)) & ~((zpu32)(ZP_DMA_UNIT - 1));

    while (done < total) {
        zpu32 chunk = total - done;
        zpu8  units;
        if (chunk > ZP_DMA_MAX_XFER) chunk = ZP_DMA_MAX_XFER;
        units = (zpu8)(chunk / ZP_DMA_UNIT);
        /* ConstRepeat: source low byte carries the constant value. */
        zp_dma_wait(chan);
        zp_dma_kick(ZP_DMA_CONST_REPEAT, chan, (zpaddr)value, dst + done,
                    (zpu8)(units - 1), 1, 0);
        done += chunk;
    }
    zp_dma_wait(chan);
}

#endif /* ZEROPOINT_ZP_DMA_H */
