/*
 * zp_ppu.h - PPU window access and DMA-based microcode/asset upload.
 *
 * The PPU has its own 64 KB address space, reached from the CPU through the
 * bank $B0 window (ZP_PPU_WIN_BASE).  PPU-internal I/O lives at low offsets;
 * the framebuffer occupies $E000-$FFFF.  To get PPU microcode, tiles, and
 * palettes into that space, DMA them from CPU ROM into the window.
 */
#ifndef ZEROPOINT_ZP_PPU_H
#define ZEROPOINT_ZP_PPU_H

#include "zp_types.h"
#include "zp_mmio.h"
#include "zp_dma.h"

/* CPU address of a PPU-internal offset. */
#define ZP_PPU(off)       (ZP_PPU_WIN_BASE + (zpu16)(off))

/* PPU-internal offsets (see CLAUDE.md "PPU Memory-Mapped I/O"). */
#define ZP_PPU_VOC        0x00F0   /* VOC control registers ($F0-$FF)   */
#define ZP_PPU_PIXEL      0x0100   /* pixel drawing block               */
#define ZP_PPU_TILE       0x0200   /* tile drawing block                */
#define ZP_PPU_TILEDEF    0x0300   /* tile definition buffer            */
#define ZP_PPU_FB         0xE000   /* framebuffer (8 KB rolling)        */
#define ZP_PPU_CODE       0x0400   /* conventional microcode load base  */

/* VOC registers, as CPU addresses through the window. */
#define ZP_VOC_RENDMODE   ZP_PPU(ZP_PPU_VOC + 0x00)  /* CRHVPLIW bits */
#define ZP_VOC_PAL_LO     ZP_PPU(ZP_PPU_VOC + 0x01)
#define ZP_VOC_PAL_HI     ZP_PPU(ZP_PPU_VOC + 0x02)
#define ZP_VOC_AUTOROLL   ZP_PPU(ZP_PPU_VOC + 0x0B)

/* Start / reset the PPU core via the $D8 control block. */
ZP_INLINE void zp_ppu_start(ZP_VOID) { zp_poke8(ZP_PPU_CTRL, ZP_PPU_CTRL_START); }
ZP_INLINE void zp_ppu_reset(ZP_VOID) { zp_poke8(ZP_PPU_CTRL, ZP_PPU_CTRL_RESET); }

/*
 * Upload a blob from CPU ROM/RAM into the PPU window at ppu_off.
 * Typical use: zp_ppu_upload(gfx_bin, ZP_PPU_CODE, gfx_bin_len, 0).
 */
ZP_INLINE zpu16 zp_ppu_upload(zpaddr src, zpu16 ppu_off, zpu32 len, zpu8 chan)
{
    return zp_dma_load(src, ZP_PPU(ppu_off), len, chan);
}

/* Load PPU microcode then start the core. */
ZP_INLINE void zp_ppu_boot(zpaddr code_src, zpu32 code_len, zpu8 chan)
{
    zp_ppu_reset();
    zp_ppu_upload(code_src, ZP_PPU_CODE, code_len, chan);
    zp_ppu_start();
}

#endif /* ZEROPOINT_ZP_PPU_H */
